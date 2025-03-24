#include "context.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "controllers/public/window_controller.hpp"
#include "loaders/public/glad_loader.hpp"
#include "loaders/public/imgui_loader.hpp"
#include "loaders/public/shader_loader.hpp"

#include <iostream>

namespace {
    void processInput(GLFWwindow *window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }
}

namespace Prism::Context {
    void Context::RunEngine() {

        Controllers::WindowController windowController;
        windowController.Initialize();

        auto window = windowController.GetWindow();

        Loaders::GladLoader gladLoader;
        gladLoader();

        Loaders::ImGuiLoader imGuiLoader;
        imGuiLoader(window);
        
        Loaders::ShaderLoader shaderLoader;
        auto shaderProgramOpt = shaderLoader("basic.vert", "basic.frag");
        if (!shaderProgramOpt) {
            throw std::runtime_error("Couldn't load shaders!");
        }
        auto shaderProgram = *shaderProgramOpt;
        
        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
            -0.5f, -0.5f, 0.0f, // left  
                0.5f, -0.5f, 0.0f, // right 
                0.0f,  0.5f, 0.0f  // top   
        }; 

        unsigned int VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0); 

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0); 

        // ImGui variables
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);
        float triangle_color[3] = { 1.0f, 0.5f, 0.2f };

        // render loop
        // -----------
        while (!glfwWindowShouldClose(window))
        {
            // input
            // -----
            processInput(window);

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 1. Show the ImGui demo window
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            // 2. Show a simple window that we create ourselves
            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, ImGui!"); // Create a window called "Hello, ImGui!" and append into it.

                ImGui::Text("This is a simple ImGui window."); // Display some text
                ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
                ImGui::Checkbox("Another Window", &show_another_window);

                ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::ColorEdit3("Triangle Color", triangle_color); // Edit 3 floats representing a color
                ImGui::ColorEdit4("Clear Color", (float*)&clear_color); // Edit 4 floats representing a color

                if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }

            // 3. Show another simple window.
            if (show_another_window)
            {
                ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me"))
                    show_another_window = false;
                ImGui::End();
            }

            // render
            // ------
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);

            // Update the fragment shader color based on ImGui control
            const char *fragmentShaderDynamicSource = "#version 330 core\n"
                "out vec4 FragColor;\n"
                "void main()\n"
                "{\n"
                "   FragColor = vec4(%ff, %ff, %ff, 1.0f);\n"
                "}\n\0";
            
            char fragmentShaderBuffer[512];
            sprintf(fragmentShaderBuffer, fragmentShaderDynamicSource, 
                    triangle_color[0], triangle_color[1], triangle_color[2]);
                    
            // We'd normally recreate the shader here but for simplicity we'll just use the original one

            // draw our triangle
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            // Render ImGui
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderProgram);

        glfwTerminate();
    }
};