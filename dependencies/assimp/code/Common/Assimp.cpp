/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2025, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/
/** @file  Assimp.cpp
 *  @brief Implementation of the Plain-C API
 */

#include <assimp/BaseImporter.h>
#include <assimp/Exceptional.h>
#include <assimp/GenericProperty.h>
#include <assimp/cimport.h>
#include <assimp/importerdesc.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>

#include "CApi/CInterfaceIOWrapper.h"
#include "Importer.h"
#include "ScenePrivate.h"

#include <list>

// ------------------------------------------------------------------------------------------------
#ifndef ASSIMP_BUILD_SINGLETHREADED
#include <mutex>
#include <thread>
#endif
// ------------------------------------------------------------------------------------------------
using namespace Assimp;

namespace Assimp {
// underlying structure for aiPropertyStore
typedef BatchLoader::PropertyMap PropertyMap;

#if defined(__has_warning)
#if __has_warning("-Wordered-compare-function-pointers")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wordered-compare-function-pointers"
#endif
#endif

/** Stores the LogStream objects for all active C log streams */
struct mpred {
    bool operator()(const aiLogStream &s0, const aiLogStream &s1) const {
        return s0.callback < s1.callback && s0.user < s1.user;
    }
};

#if defined(__has_warning)
#if __has_warning("-Wordered-compare-function-pointers")
#pragma GCC diagnostic pop
#endif
#endif
typedef std::map<aiLogStream, Assimp::LogStream *, mpred> LogStreamMap;

/** Stores the LogStream objects allocated by #aiGetPredefinedLogStream */
typedef std::list<Assimp::LogStream *> PredefLogStreamMap;

/** Local storage of all active log streams */
static LogStreamMap gActiveLogStreams;

/** Local storage of LogStreams allocated by #aiGetPredefinedLogStream */
static PredefLogStreamMap gPredefinedStreams;

/** Error message of the last failed import process */
static std::string gLastErrorString;

/** Verbose logging active or not? */
static aiBool gVerboseLogging = false;

/** will return all registered importers. */
void GetImporterInstanceList(std::vector<BaseImporter *> &out);

/** will delete all registered importers. */
void DeleteImporterInstanceList(std::vector<BaseImporter *> &out);
} // namespace Assimp

#ifndef ASSIMP_BUILD_SINGLETHREADED
/** Global mutex to manage the access to the log-stream map */
static std::mutex gLogStreamMutex;
#endif

// ------------------------------------------------------------------------------------------------
// Custom LogStream implementation for the C-API
class LogToCallbackRedirector final : public LogStream {
public:
    explicit LogToCallbackRedirector(const aiLogStream &s) :
            mStream(s) {
        ai_assert(nullptr != s.callback);
    }

    ~LogToCallbackRedirector() {
#ifndef ASSIMP_BUILD_SINGLETHREADED
        std::lock_guard<std::mutex> lock(gLogStreamMutex);
#endif
        // (HACK) Check whether the 'stream.user' pointer points to a
        // custom LogStream allocated by #aiGetPredefinedLogStream.
        // In this case, we need to delete it, too. Of course, this
        // might cause strange problems, but the chance is quite low.

        PredefLogStreamMap::iterator it = std::find(gPredefinedStreams.begin(),
                gPredefinedStreams.end(), (Assimp::LogStream *)mStream.user);

        if (it != gPredefinedStreams.end()) {
            delete *it;
            gPredefinedStreams.erase(it);
        }
    }

    /** @copydoc LogStream::write */
    void write(const char *message) {
        mStream.callback(message, mStream.user);
    }

private:
    const aiLogStream &mStream;
};

// ------------------------------------------------------------------------------------------------
void ReportSceneNotFoundError() {
    ASSIMP_LOG_ERROR("Unable to find the Assimp::Importer for this aiScene. "
                     "The C-API does not accept scenes produced by the C++ API and vice versa");

    ai_assert(false);
}

// ------------------------------------------------------------------------------------------------
// Reads the given file and returns its content.
const aiScene *aiImportFile(const char *pFile, unsigned int pFlags) {
    return aiImportFileEx(pFile, pFlags, nullptr);
}

// ------------------------------------------------------------------------------------------------
const aiScene *aiImportFileEx(const char *pFile, unsigned int pFlags, aiFileIO *pFS) {
    return aiImportFileExWithProperties(pFile, pFlags, pFS, nullptr);
}

// ------------------------------------------------------------------------------------------------
const aiScene *aiImportFileExWithProperties(const char *pFile, unsigned int pFlags,
        aiFileIO *pFS, const aiPropertyStore *props) {
    ai_assert(nullptr != pFile);

    const aiScene *scene = nullptr;
    ASSIMP_BEGIN_EXCEPTION_REGION();

    // create an Importer for this file
    Assimp::Importer *imp = new Assimp::Importer();

    // copy properties
    if (props) {
        const PropertyMap *pp = reinterpret_cast<const PropertyMap *>(props);
        ImporterPimpl *pimpl = imp->Pimpl();
        pimpl->mIntProperties = pp->ints;
        pimpl->mFloatProperties = pp->floats;
        pimpl->mStringProperties = pp->strings;
        pimpl->mMatrixProperties = pp->matrices;
    }
    // setup a custom IO system if necessary
    if (pFS) {
        imp->SetIOHandler(new CIOSystemWrapper(pFS));
    }

    // and have it read the file
    scene = imp->ReadFile(pFile, pFlags);

    // if succeeded, store the importer in the scene and keep it alive
    if (scene) {
        ScenePrivateData *priv = const_cast<ScenePrivateData *>(ScenePriv(scene));
        priv->mOrigImporter = imp;
    } else {
        // if failed, extract error code and destroy the import
        gLastErrorString = imp->GetErrorString();
        delete imp;
    }

    // return imported data. If the import failed the pointer is nullptr anyways
    ASSIMP_END_EXCEPTION_REGION(const aiScene *);

    return scene;
}

// ------------------------------------------------------------------------------------------------
const aiScene *aiImportFileFromMemory(
        const char *pBuffer,
        unsigned int pLength,
        unsigned int pFlags,
        const char *pHint) {
    return aiImportFileFromMemoryWithProperties(pBuffer, pLength, pFlags, pHint, nullptr);
}

// ------------------------------------------------------------------------------------------------
const aiScene *aiImportFileFromMemoryWithProperties(
        const char *pBuffer,
        unsigned int pLength,
        unsigned int pFlags,
        const char *pHint,
        const aiPropertyStore *props) {
    if (pBuffer == nullptr) {
        return nullptr;
    }

    if (pLength == 0u) {
        return nullptr;
    }

    const aiScene *scene = nullptr;
    ASSIMP_BEGIN_EXCEPTION_REGION();

    // create an Importer for this file
    Assimp::Importer *imp = new Assimp::Importer();

    // copy properties
    if (props) {
        const PropertyMap *pp = reinterpret_cast<const PropertyMap *>(props);
        ImporterPimpl *pimpl = imp->Pimpl();
        pimpl->mIntProperties = pp->ints;
        pimpl->mFloatProperties = pp->floats;
        pimpl->mStringProperties = pp->strings;
        pimpl->mMatrixProperties = pp->matrices;
    }

    // and have it read the file from the memory buffer
    scene = imp->ReadFileFromMemory(pBuffer, pLength, pFlags, pHint);

    // if succeeded, store the importer in the scene and keep it alive
    if (scene) {
        ScenePrivateData *priv = const_cast<ScenePrivateData *>(ScenePriv(scene));
        priv->mOrigImporter = imp;
    } else {
        // if failed, extract error code and destroy the import
        gLastErrorString = imp->GetErrorString();
        delete imp;
    }
    // return imported data. If the import failed the pointer is nullptr anyways
    ASSIMP_END_EXCEPTION_REGION(const aiScene *);
    return scene;
}

// ------------------------------------------------------------------------------------------------
// Releases all resources associated with the given import process.
void aiReleaseImport(const aiScene *pScene) {
    if (!pScene) {
        return;
    }

    ASSIMP_BEGIN_EXCEPTION_REGION();

    // find the importer associated with this data
    const ScenePrivateData *priv = ScenePriv(pScene);
    if (!priv || !priv->mOrigImporter) {
        delete pScene;
    } else {
        // deleting the Importer also deletes the scene
        // Note: the reason that this is not written as 'delete priv->mOrigImporter'
        // is a suspected bug in gcc 4.4+ (http://gcc.gnu.org/bugzilla/show_bug.cgi?id=52339)
        Importer *importer = priv->mOrigImporter;
        delete importer;
    }

    ASSIMP_END_EXCEPTION_REGION(void);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API const aiScene *aiApplyPostProcessing(const aiScene *pScene,
        unsigned int pFlags) {
    const aiScene *sc = nullptr;

    ASSIMP_BEGIN_EXCEPTION_REGION();

    // find the importer associated with this data
    const ScenePrivateData *priv = ScenePriv(pScene);
    if (!priv || !priv->mOrigImporter) {
        ReportSceneNotFoundError();
        return nullptr;
    }

    sc = priv->mOrigImporter->ApplyPostProcessing(pFlags);

    if (!sc) {
        aiReleaseImport(pScene);
        return nullptr;
    }

    ASSIMP_END_EXCEPTION_REGION(const aiScene *);
    return sc;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API const aiScene *aiApplyCustomizedPostProcessing(const aiScene *scene,
        BaseProcess *process,
        bool requestValidation) {
    const aiScene *sc(nullptr);

    ASSIMP_BEGIN_EXCEPTION_REGION();

    // find the importer associated with this data
    const ScenePrivateData *priv = ScenePriv(scene);
    if (nullptr == priv || nullptr == priv->mOrigImporter) {
        ReportSceneNotFoundError();
        return nullptr;
    }

    sc = priv->mOrigImporter->ApplyCustomizedPostProcessing(process, requestValidation);

    if (!sc) {
        aiReleaseImport(scene);
        return nullptr;
    }

    ASSIMP_END_EXCEPTION_REGION(const aiScene *);

    return sc;
}

// ------------------------------------------------------------------------------------------------
void CallbackToLogRedirector(const char *msg, char *dt) {
    ai_assert(nullptr != msg);
    ai_assert(nullptr != dt);
    LogStream *stream = (LogStream *)dt;
    if (stream != nullptr) {
        stream->write(msg);
    }
}

static LogStream *DefaultStream = nullptr;

// ------------------------------------------------------------------------------------------------
ASSIMP_API aiLogStream aiGetPredefinedLogStream(aiDefaultLogStream pStream, const char *file) {
    aiLogStream sout;

    ASSIMP_BEGIN_EXCEPTION_REGION();
    if (DefaultStream == nullptr) {
        DefaultStream = LogStream::createDefaultStream(pStream, file);
    }

    if (!DefaultStream) {
        sout.callback = nullptr;
        sout.user = nullptr;
    } else {
        sout.callback = &CallbackToLogRedirector;
        sout.user = (char *)DefaultStream;
    }
    gPredefinedStreams.push_back(DefaultStream);
    ASSIMP_END_EXCEPTION_REGION(aiLogStream);
    return sout;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiAttachLogStream(const aiLogStream *stream) {
    ASSIMP_BEGIN_EXCEPTION_REGION();

#ifndef ASSIMP_BUILD_SINGLETHREADED
    std::lock_guard<std::mutex> lock(gLogStreamMutex);
#endif

    LogStream *lg = new LogToCallbackRedirector(*stream);
    gActiveLogStreams[*stream] = lg;

    if (DefaultLogger::isNullLogger()) {
        DefaultLogger::create(nullptr, (gVerboseLogging == AI_TRUE ? Logger::VERBOSE : Logger::NORMAL));
    }
    DefaultLogger::get()->attachStream(lg);
    ASSIMP_END_EXCEPTION_REGION(void);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API aiReturn aiDetachLogStream(const aiLogStream *stream) {
    ASSIMP_BEGIN_EXCEPTION_REGION();

#ifndef ASSIMP_BUILD_SINGLETHREADED
    std::lock_guard<std::mutex> lock(gLogStreamMutex);
#endif
    // find the log-stream associated with this data
    LogStreamMap::iterator it = gActiveLogStreams.find(*stream);
    // it should be there... else the user is playing fools with us
    if (it == gActiveLogStreams.end()) {
        return AI_FAILURE;
    }
    DefaultLogger::get()->detachStream(it->second);
    delete it->second;

    if ((Assimp::LogStream *)stream->user == DefaultStream) {
        DefaultStream = nullptr;
    }

    gActiveLogStreams.erase(it);

    if (gActiveLogStreams.empty()) {
        DefaultLogger::kill();
    }
    ASSIMP_END_EXCEPTION_REGION(aiReturn);
    return AI_SUCCESS;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiDetachAllLogStreams(void) {
    ASSIMP_BEGIN_EXCEPTION_REGION();
#ifndef ASSIMP_BUILD_SINGLETHREADED
    std::lock_guard<std::mutex> lock(gLogStreamMutex);
#endif
    Logger *logger(DefaultLogger::get());
    if (nullptr == logger) {
        return;
    }

    for (LogStreamMap::iterator it = gActiveLogStreams.begin(); it != gActiveLogStreams.end(); ++it) {
        logger->detachStream(it->second);
        delete it->second;
    }
    gActiveLogStreams.clear();
    DefaultLogger::kill();

    ASSIMP_END_EXCEPTION_REGION(void);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiEnableVerboseLogging(aiBool d) {
    if (!DefaultLogger::isNullLogger()) {
        DefaultLogger::get()->setLogSeverity((d == AI_TRUE ? Logger::VERBOSE : Logger::NORMAL));
    }
    gVerboseLogging = d;
}

// ------------------------------------------------------------------------------------------------
// Returns the error text of the last failed import process.
const char *aiGetErrorString() {
    return gLastErrorString.c_str();
}

// -----------------------------------------------------------------------------------------------
// Return the description of a importer given its index
const aiImporterDesc *aiGetImportFormatDescription(size_t pIndex) {
    return Importer().GetImporterInfo(pIndex);
}

// -----------------------------------------------------------------------------------------------
// Return the number of importers
size_t aiGetImportFormatCount(void) {
    return Importer().GetImporterCount();
}

// ------------------------------------------------------------------------------------------------
// Returns the error text of the last failed import process.
aiBool aiIsExtensionSupported(const char *szExtension) {
    ai_assert(nullptr != szExtension);
    aiBool candoit = AI_FALSE;
    ASSIMP_BEGIN_EXCEPTION_REGION();

    // FIXME: no need to create a temporary Importer instance just for that ..
    Assimp::Importer tmp;
    candoit = tmp.IsExtensionSupported(std::string(szExtension)) ? AI_TRUE : AI_FALSE;

    ASSIMP_END_EXCEPTION_REGION(aiBool);
    return candoit;
}

// ------------------------------------------------------------------------------------------------
// Get a list of all file extensions supported by ASSIMP
void aiGetExtensionList(aiString *szOut) {
    ai_assert(nullptr != szOut);
    ASSIMP_BEGIN_EXCEPTION_REGION();

    // FIXME: no need to create a temporary Importer instance just for that ..
    Assimp::Importer tmp;
    tmp.GetExtensionList(*szOut);

    ASSIMP_END_EXCEPTION_REGION(void);
}

// ------------------------------------------------------------------------------------------------
// Get the memory requirements for a particular import.
void aiGetMemoryRequirements(const C_STRUCT aiScene *pIn,
        C_STRUCT aiMemoryInfo *in) {
    ASSIMP_BEGIN_EXCEPTION_REGION();

    // find the importer associated with this data
    const ScenePrivateData *priv = ScenePriv(pIn);
    if (!priv || !priv->mOrigImporter) {
        ReportSceneNotFoundError();
        return;
    }

    return priv->mOrigImporter->GetMemoryRequirements(*in);
    ASSIMP_END_EXCEPTION_REGION(void);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API const C_STRUCT aiTexture *aiGetEmbeddedTexture(const C_STRUCT aiScene *pIn, const char *filename) {
    return pIn->GetEmbeddedTexture(filename);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API aiPropertyStore *aiCreatePropertyStore(void) {
    return reinterpret_cast<aiPropertyStore *>(new PropertyMap());
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiReleasePropertyStore(aiPropertyStore *p) {
    delete reinterpret_cast<PropertyMap *>(p);
}

// ------------------------------------------------------------------------------------------------
// Importer::SetPropertyInteger
ASSIMP_API void aiSetImportPropertyInteger(aiPropertyStore *p, const char *szName, int value) {
    ASSIMP_BEGIN_EXCEPTION_REGION();
    PropertyMap *pp = reinterpret_cast<PropertyMap *>(p);
    SetGenericProperty<int>(pp->ints, szName, value);
    ASSIMP_END_EXCEPTION_REGION(void);
}

// ------------------------------------------------------------------------------------------------
// Importer::SetPropertyFloat
ASSIMP_API void aiSetImportPropertyFloat(aiPropertyStore *p, const char *szName, ai_real value) {
    ASSIMP_BEGIN_EXCEPTION_REGION();
    PropertyMap *pp = reinterpret_cast<PropertyMap *>(p);
    SetGenericProperty<ai_real>(pp->floats, szName, value);
    ASSIMP_END_EXCEPTION_REGION(void);
}

// ------------------------------------------------------------------------------------------------
// Importer::SetPropertyString
ASSIMP_API void aiSetImportPropertyString(aiPropertyStore *p, const char *szName,
        const C_STRUCT aiString *st) {
    if (!st) {
        return;
    }
    ASSIMP_BEGIN_EXCEPTION_REGION();
    PropertyMap *pp = reinterpret_cast<PropertyMap *>(p);
    SetGenericProperty<std::string>(pp->strings, szName, std::string(st->C_Str()));
    ASSIMP_END_EXCEPTION_REGION(void);
}

// ------------------------------------------------------------------------------------------------
// Importer::SetPropertyMatrix
ASSIMP_API void aiSetImportPropertyMatrix(aiPropertyStore *p, const char *szName,
        const C_STRUCT aiMatrix4x4 *mat) {
    if (!mat) {
        return;
    }
    ASSIMP_BEGIN_EXCEPTION_REGION();
    PropertyMap *pp = reinterpret_cast<PropertyMap *>(p);
    SetGenericProperty<aiMatrix4x4>(pp->matrices, szName, *mat);
    ASSIMP_END_EXCEPTION_REGION(void);
}

// ------------------------------------------------------------------------------------------------
// Rotation matrix to quaternion
ASSIMP_API void aiCreateQuaternionFromMatrix(aiQuaternion *quat, const aiMatrix3x3 *mat) {
    ai_assert(nullptr != quat);
    ai_assert(nullptr != mat);
    *quat = aiQuaternion(*mat);
}

// ------------------------------------------------------------------------------------------------
// Matrix decomposition
ASSIMP_API void aiDecomposeMatrix(const aiMatrix4x4 *mat, aiVector3D *scaling,
        aiQuaternion *rotation,
        aiVector3D *position) {
    ai_assert(nullptr != rotation);
    ai_assert(nullptr != position);
    ai_assert(nullptr != scaling);
    ai_assert(nullptr != mat);
    mat->Decompose(*scaling, *rotation, *position);
}

// ------------------------------------------------------------------------------------------------
// Matrix transpose
ASSIMP_API void aiTransposeMatrix3(aiMatrix3x3 *mat) {
    ai_assert(nullptr != mat);
    mat->Transpose();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiTransposeMatrix4(aiMatrix4x4 *mat) {
    ai_assert(nullptr != mat);
    mat->Transpose();
}

// ------------------------------------------------------------------------------------------------
// Vector transformation
ASSIMP_API void aiTransformVecByMatrix3(aiVector3D *vec,
        const aiMatrix3x3 *mat) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != vec);
    *vec *= (*mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiTransformVecByMatrix4(aiVector3D *vec,
        const aiMatrix4x4 *mat) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != vec);

    *vec *= (*mat);
}

// ------------------------------------------------------------------------------------------------
// Matrix multiplication
ASSIMP_API void aiMultiplyMatrix4(
        aiMatrix4x4 *dst,
        const aiMatrix4x4 *src) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != src);
    *dst = (*dst) * (*src);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMultiplyMatrix3(
        aiMatrix3x3 *dst,
        const aiMatrix3x3 *src) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != src);
    *dst = (*dst) * (*src);
}

// ------------------------------------------------------------------------------------------------
// Matrix identity
ASSIMP_API void aiIdentityMatrix3(
        aiMatrix3x3 *mat) {
    ai_assert(nullptr != mat);
    *mat = aiMatrix3x3();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiIdentityMatrix4(
        aiMatrix4x4 *mat) {
    ai_assert(nullptr != mat);
    *mat = aiMatrix4x4();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API C_STRUCT const aiImporterDesc *aiGetImporterDesc(const char *extension) {
    if (nullptr == extension) {
        return nullptr;
    }
    const aiImporterDesc *desc(nullptr);
    std::vector<BaseImporter *> out;
    GetImporterInstanceList(out);
    for (size_t i = 0; i < out.size(); ++i) {
        if (0 == strncmp(out[i]->GetInfo()->mFileExtensions, extension, strlen(extension))) {
            desc = out[i]->GetInfo();
            break;
        }
    }

    DeleteImporterInstanceList(out);

    return desc;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiVector2AreEqual(
        const C_STRUCT aiVector2D *a,
        const C_STRUCT aiVector2D *b) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return *a == *b;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiVector2AreEqualEpsilon(
        const C_STRUCT aiVector2D *a,
        const C_STRUCT aiVector2D *b,
        const float epsilon) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return a->Equal(*b, epsilon);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector2Add(
        C_STRUCT aiVector2D *dst,
        const C_STRUCT aiVector2D *src) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != src);
    *dst = *dst + *src;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector2Subtract(
        C_STRUCT aiVector2D *dst,
        const C_STRUCT aiVector2D *src) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != src);
    *dst = *dst - *src;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector2Scale(
        C_STRUCT aiVector2D *dst,
        const float s) {
    ai_assert(nullptr != dst);
    *dst *= s;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector2SymMul(
        C_STRUCT aiVector2D *dst,
        const C_STRUCT aiVector2D *other) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != other);
    *dst = dst->SymMul(*other);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector2DivideByScalar(
        C_STRUCT aiVector2D *dst,
        const float s) {
    ai_assert(nullptr != dst);
    *dst /= s;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector2DivideByVector(
        C_STRUCT aiVector2D *dst,
        C_STRUCT aiVector2D *v) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != v);
    *dst = *dst / *v;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API ai_real aiVector2Length(
        const C_STRUCT aiVector2D *v) {
    ai_assert(nullptr != v);
    return v->Length();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API ai_real aiVector2SquareLength(
        const C_STRUCT aiVector2D *v) {
    ai_assert(nullptr != v);
    return v->SquareLength();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector2Negate(
        C_STRUCT aiVector2D *dst) {
    ai_assert(nullptr != dst);
    *dst = -(*dst);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API ai_real aiVector2DotProduct(
        const C_STRUCT aiVector2D *a,
        const C_STRUCT aiVector2D *b) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return (*a) * (*b);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector2Normalize(
        C_STRUCT aiVector2D *v) {
    ai_assert(nullptr != v);
    v->Normalize();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiVector3AreEqual(
        const C_STRUCT aiVector3D *a,
        const C_STRUCT aiVector3D *b) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return *a == *b;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiVector3AreEqualEpsilon(
        const C_STRUCT aiVector3D *a,
        const C_STRUCT aiVector3D *b,
        const float epsilon) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return a->Equal(*b, epsilon);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiVector3LessThan(
        const C_STRUCT aiVector3D *a,
        const C_STRUCT aiVector3D *b) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return *a < *b;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector3Add(
        C_STRUCT aiVector3D *dst,
        const C_STRUCT aiVector3D *src) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != src);
    *dst = *dst + *src;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector3Subtract(
        C_STRUCT aiVector3D *dst,
        const C_STRUCT aiVector3D *src) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != src);
    *dst = *dst - *src;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector3Scale(
        C_STRUCT aiVector3D *dst,
        const float s) {
    ai_assert(nullptr != dst);
    *dst *= s;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector3SymMul(
        C_STRUCT aiVector3D *dst,
        const C_STRUCT aiVector3D *other) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != other);
    *dst = dst->SymMul(*other);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector3DivideByScalar(
        C_STRUCT aiVector3D *dst, const float s) {
    ai_assert(nullptr != dst);
    *dst /= s;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector3DivideByVector(
        C_STRUCT aiVector3D *dst,
        C_STRUCT aiVector3D *v) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != v);
    *dst = *dst / *v;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API ai_real aiVector3Length(
        const C_STRUCT aiVector3D *v) {
    ai_assert(nullptr != v);
    return v->Length();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API ai_real aiVector3SquareLength(
        const C_STRUCT aiVector3D *v) {
    ai_assert(nullptr != v);
    return v->SquareLength();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector3Negate(
        C_STRUCT aiVector3D *dst) {
    ai_assert(nullptr != dst);
    *dst = -(*dst);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API ai_real aiVector3DotProduct(
        const C_STRUCT aiVector3D *a,
        const C_STRUCT aiVector3D *b) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return (*a) * (*b);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector3CrossProduct(
        C_STRUCT aiVector3D *dst,
        const C_STRUCT aiVector3D *a,
        const C_STRUCT aiVector3D *b) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    *dst = *a ^ *b;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector3Normalize(
        C_STRUCT aiVector3D *v) {
    ai_assert(nullptr != v);
    v->Normalize();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector3NormalizeSafe(
        C_STRUCT aiVector3D *v) {
    ai_assert(nullptr != v);
    v->NormalizeSafe();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiVector3RotateByQuaternion(
        C_STRUCT aiVector3D *v,
        const C_STRUCT aiQuaternion *q) {
    ai_assert(nullptr != v);
    ai_assert(nullptr != q);
    *v = q->Rotate(*v);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix3FromMatrix4(
        C_STRUCT aiMatrix3x3 *dst,
        const C_STRUCT aiMatrix4x4 *mat) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != mat);
    *dst = aiMatrix3x3(*mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix3FromQuaternion(
        C_STRUCT aiMatrix3x3 *mat,
        const C_STRUCT aiQuaternion *q) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != q);
    *mat = q->GetMatrix();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiMatrix3AreEqual(
        const C_STRUCT aiMatrix3x3 *a,
        const C_STRUCT aiMatrix3x3 *b) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return *a == *b;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiMatrix3AreEqualEpsilon(
        const C_STRUCT aiMatrix3x3 *a,
        const C_STRUCT aiMatrix3x3 *b,
        const float epsilon) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return a->Equal(*b, epsilon);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix3Inverse(C_STRUCT aiMatrix3x3 *mat) {
    ai_assert(nullptr != mat);
    mat->Inverse();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API ai_real aiMatrix3Determinant(const C_STRUCT aiMatrix3x3 *mat) {
    ai_assert(nullptr != mat);
    return mat->Determinant();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix3RotationZ(
        C_STRUCT aiMatrix3x3 *mat,
        const float angle) {
    ai_assert(nullptr != mat);
    aiMatrix3x3::RotationZ(angle, *mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix3FromRotationAroundAxis(
        C_STRUCT aiMatrix3x3 *mat,
        const C_STRUCT aiVector3D *axis,
        const float angle) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != axis);
    aiMatrix3x3::Rotation(angle, *axis, *mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix3Translation(
        C_STRUCT aiMatrix3x3 *mat,
        const C_STRUCT aiVector2D *translation) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != translation);
    aiMatrix3x3::Translation(*translation, *mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix3FromTo(
        C_STRUCT aiMatrix3x3 *mat,
        const C_STRUCT aiVector3D *from,
        const C_STRUCT aiVector3D *to) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != from);
    ai_assert(nullptr != to);
    aiMatrix3x3::FromToMatrix(*from, *to, *mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4FromMatrix3(
        C_STRUCT aiMatrix4x4 *dst,
        const C_STRUCT aiMatrix3x3 *mat) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != mat);
    *dst = aiMatrix4x4(*mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4FromScalingQuaternionPosition(
        C_STRUCT aiMatrix4x4 *mat,
        const C_STRUCT aiVector3D *scaling,
        const C_STRUCT aiQuaternion *rotation,
        const C_STRUCT aiVector3D *position) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != scaling);
    ai_assert(nullptr != rotation);
    ai_assert(nullptr != position);
    *mat = aiMatrix4x4(*scaling, *rotation, *position);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4Add(
        C_STRUCT aiMatrix4x4 *dst,
        const C_STRUCT aiMatrix4x4 *src) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != src);
    *dst = *dst + *src;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiMatrix4AreEqual(
        const C_STRUCT aiMatrix4x4 *a,
        const C_STRUCT aiMatrix4x4 *b) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return *a == *b;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiMatrix4AreEqualEpsilon(
        const C_STRUCT aiMatrix4x4 *a,
        const C_STRUCT aiMatrix4x4 *b,
        const float epsilon) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return a->Equal(*b, epsilon);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4Inverse(C_STRUCT aiMatrix4x4 *mat) {
    ai_assert(nullptr != mat);
    mat->Inverse();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API ai_real aiMatrix4Determinant(const C_STRUCT aiMatrix4x4 *mat) {
    ai_assert(nullptr != mat);
    return mat->Determinant();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiMatrix4IsIdentity(const C_STRUCT aiMatrix4x4 *mat) {
    ai_assert(nullptr != mat);
    return mat->IsIdentity();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4DecomposeIntoScalingEulerAnglesPosition(
        const C_STRUCT aiMatrix4x4 *mat,
        C_STRUCT aiVector3D *scaling,
        C_STRUCT aiVector3D *rotation,
        C_STRUCT aiVector3D *position) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != scaling);
    ai_assert(nullptr != rotation);
    ai_assert(nullptr != position);
    mat->Decompose(*scaling, *rotation, *position);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4DecomposeIntoScalingAxisAnglePosition(
        const C_STRUCT aiMatrix4x4 *mat,
        C_STRUCT aiVector3D *scaling,
        C_STRUCT aiVector3D *axis,
        ai_real *angle,
        C_STRUCT aiVector3D *position) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != scaling);
    ai_assert(nullptr != axis);
    ai_assert(nullptr != angle);
    ai_assert(nullptr != position);
    mat->Decompose(*scaling, *axis, *angle, *position);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4DecomposeNoScaling(
        const C_STRUCT aiMatrix4x4 *mat,
        C_STRUCT aiQuaternion *rotation,
        C_STRUCT aiVector3D *position) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != rotation);
    ai_assert(nullptr != position);
    mat->DecomposeNoScaling(*rotation, *position);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4FromEulerAngles(
        C_STRUCT aiMatrix4x4 *mat,
        float x, float y, float z) {
    ai_assert(nullptr != mat);
    mat->FromEulerAnglesXYZ(x, y, z);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4RotationX(
        C_STRUCT aiMatrix4x4 *mat,
        const float angle) {
    ai_assert(nullptr != mat);
    aiMatrix4x4::RotationX(angle, *mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4RotationY(
        C_STRUCT aiMatrix4x4 *mat,
        const float angle) {
    ai_assert(nullptr != mat);
    aiMatrix4x4::RotationY(angle, *mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4RotationZ(
        C_STRUCT aiMatrix4x4 *mat,
        const float angle) {
    ai_assert(nullptr != mat);
    aiMatrix4x4::RotationZ(angle, *mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4FromRotationAroundAxis(
        C_STRUCT aiMatrix4x4 *mat,
        const C_STRUCT aiVector3D *axis,
        const float angle) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != axis);
    aiMatrix4x4::Rotation(angle, *axis, *mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4Translation(
        C_STRUCT aiMatrix4x4 *mat,
        const C_STRUCT aiVector3D *translation) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != translation);
    aiMatrix4x4::Translation(*translation, *mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4Scaling(
        C_STRUCT aiMatrix4x4 *mat,
        const C_STRUCT aiVector3D *scaling) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != scaling);
    aiMatrix4x4::Scaling(*scaling, *mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiMatrix4FromTo(
        C_STRUCT aiMatrix4x4 *mat,
        const C_STRUCT aiVector3D *from,
        const C_STRUCT aiVector3D *to) {
    ai_assert(nullptr != mat);
    ai_assert(nullptr != from);
    ai_assert(nullptr != to);
    aiMatrix4x4::FromToMatrix(*from, *to, *mat);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiQuaternionFromEulerAngles(
        C_STRUCT aiQuaternion *q,
        float x, float y, float z) {
    ai_assert(nullptr != q);
    *q = aiQuaternion(x, y, z);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiQuaternionFromAxisAngle(
        C_STRUCT aiQuaternion *q,
        const C_STRUCT aiVector3D *axis,
        const float angle) {
    ai_assert(nullptr != q);
    ai_assert(nullptr != axis);
    *q = aiQuaternion(*axis, angle);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiQuaternionFromNormalizedQuaternion(
        C_STRUCT aiQuaternion *q,
        const C_STRUCT aiVector3D *normalized) {
    ai_assert(nullptr != q);
    ai_assert(nullptr != normalized);
    *q = aiQuaternion(*normalized);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiQuaternionAreEqual(
        const C_STRUCT aiQuaternion *a,
        const C_STRUCT aiQuaternion *b) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return *a == *b;
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API int aiQuaternionAreEqualEpsilon(
        const C_STRUCT aiQuaternion *a,
        const C_STRUCT aiQuaternion *b,
        const float epsilon) {
    ai_assert(nullptr != a);
    ai_assert(nullptr != b);
    return a->Equal(*b, epsilon);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiQuaternionNormalize(
        C_STRUCT aiQuaternion *q) {
    ai_assert(nullptr != q);
    q->Normalize();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiQuaternionConjugate(
        C_STRUCT aiQuaternion *q) {
    ai_assert(nullptr != q);
    q->Conjugate();
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiQuaternionMultiply(
        C_STRUCT aiQuaternion *dst,
        const C_STRUCT aiQuaternion *q) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != q);
    *dst = (*dst) * (*q);
}

// ------------------------------------------------------------------------------------------------
ASSIMP_API void aiQuaternionInterpolate(
        C_STRUCT aiQuaternion *dst,
        const C_STRUCT aiQuaternion *start,
        const C_STRUCT aiQuaternion *end,
        const float factor) {
    ai_assert(nullptr != dst);
    ai_assert(nullptr != start);
    ai_assert(nullptr != end);
    aiQuaternion::Interpolate(*dst, *start, *end, factor);
}

// stb_image is a lightweight image loader. It is shared by:
//  - M3D import
//  - PBRT export
// Since it's a header-only library, its implementation must be instantiated in some cpp file.
// Don't scatter this task over multiple importers/exporters. Maintain it in a central place (here!).

#define ASSIMP_HAS_PBRT_EXPORT (!ASSIMP_BUILD_NO_EXPORT && !ASSIMP_BUILD_NO_PBRT_EXPORTER)
#define ASSIMP_HAS_M3D ((!ASSIMP_BUILD_NO_EXPORT && !ASSIMP_BUILD_NO_M3D_EXPORTER) || !ASSIMP_BUILD_NO_M3D_IMPORTER)

#ifndef STB_USE_HUNTER
#if ASSIMP_HAS_PBRT_EXPORT
#define ASSIMP_NEEDS_STB_IMAGE 1
#elif ASSIMP_HAS_M3D
#define ASSIMP_NEEDS_STB_IMAGE 1
#define STBI_ONLY_PNG
#endif
#endif

// Ensure all symbols are linked correctly
#if ASSIMP_NEEDS_STB_IMAGE
// Share stb_image's PNG loader with other importers/exporters instead of bringing our own copy.
#define STBI_ONLY_PNG
#ifdef ASSIMP_USE_STB_IMAGE_STATIC
#define STB_IMAGE_STATIC
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "Common/StbCommon.h"
#endif
