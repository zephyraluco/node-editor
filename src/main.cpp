#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include "imnodes.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

struct Node
{
    int id;
    int input_attr;
    int output_attr;
    std::string name;
    float value;
};

struct Link
{
    int id;
    int start_attr;
    int end_attr;
};

struct EditorState
{
    std::vector<Node> nodes;
    std::vector<Link> links;
    int next_node_id = 1;
    int next_attr_id = 100;
    int next_link_id = 1000;

    void AddNode()
    {
        Node node{};
        node.id = next_node_id++;
        node.input_attr = next_attr_id++;
        node.output_attr = next_attr_id++;
        node.name = "Node " + std::to_string(node.id);
        node.value = 0.5f;
        nodes.push_back(node);

        const float offset = static_cast<float>(nodes.size() - 1) * 140.0f;
        ImNodes::SetNodeGridSpacePos(node.id, ImVec2(80.0f + offset, 80.0f + offset * 0.3f));
    }

    bool IsInputAttribute(const int attr_id) const
    {
        for (const Node& node : nodes)
        {
            if (node.input_attr == attr_id)
            {
                return true;
            }
        }
        return false;
    }

    bool IsOutputAttribute(const int attr_id) const
    {
        for (const Node& node : nodes)
        {
            if (node.output_attr == attr_id)
            {
                return true;
            }
        }
        return false;
    }

    void TryCreateLink(int a, int b)
    {
        int start_attr = a;
        int end_attr = b;
        if (IsInputAttribute(start_attr) && IsOutputAttribute(end_attr))
        {
            std::swap(start_attr, end_attr);
        }

        if (!IsOutputAttribute(start_attr) || !IsInputAttribute(end_attr))
        {
            return;
        }

        const auto exists = std::find_if(links.begin(), links.end(), [start_attr, end_attr](const Link& link) {
            return link.start_attr == start_attr && link.end_attr == end_attr;
        });
        if (exists != links.end())
        {
            return;
        }

        Link link{};
        link.id = next_link_id++;
        link.start_attr = start_attr;
        link.end_attr = end_attr;
        links.push_back(link);
    }

    void RemoveLinkById(const int link_id)
    {
        links.erase(std::remove_if(links.begin(), links.end(), [link_id](const Link& link) { return link.id == link_id; }), links.end());
    }
};

static bool InitSdlAndGl(SDL_Window** out_window, SDL_GLContext* out_gl_context, const char** out_glsl_version)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        std::printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return false;
    }

#if defined(IMGUI_IMPL_OPENGL_ES2)
    *out_glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    *out_glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    *out_glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    *out_glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    const float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    const SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    *out_window = SDL_CreateWindow("SDL3 + ImGui + imnodes", static_cast<int>(1280 * scale), static_cast<int>(800 * scale), window_flags);
    if (*out_window == nullptr)
    {
        std::printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return false;
    }

    *out_gl_context = SDL_GL_CreateContext(*out_window);
    if (*out_gl_context == nullptr)
    {
        std::printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_MakeCurrent(*out_window, *out_gl_context);
    SDL_GL_SetSwapInterval(1);
    SDL_SetWindowPosition(*out_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(*out_window);
    return true;
}

int main(int, char**)
{
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    const char* glsl_version = nullptr;
    if (!InitSdlAndGl(&window, &gl_context, &glsl_version))
    {
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImNodes::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImNodes::StyleColorsDark();

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    EditorState editor;
    editor.AddNode();
    editor.AddNode();

    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
            {
                done = true;
            }
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
            {
                done = true;
            }
        }

        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Node Graph");
        if (ImGui::Button("Add Node"))
        {
            editor.AddNode();
        }
        ImGui::SameLine();
        ImGui::Text("Nodes: %d  Links: %d", static_cast<int>(editor.nodes.size()), static_cast<int>(editor.links.size()));

        ImNodes::BeginNodeEditor();

        for (Node& node : editor.nodes)
        {
            ImNodes::BeginNode(node.id);

            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted(node.name.c_str());
            ImNodes::EndNodeTitleBar();

            ImNodes::BeginInputAttribute(node.input_attr);
            ImGui::Text("In");
            ImNodes::EndInputAttribute();

            ImGui::PushItemWidth(120.0f);
            ImGui::SliderFloat(("##value_" + std::to_string(node.id)).c_str(), &node.value, 0.0f, 1.0f, "value %.2f");
            ImGui::PopItemWidth();

            ImNodes::BeginOutputAttribute(node.output_attr);
            ImGui::Indent(40.0f);
            ImGui::Text("Out");
            ImNodes::EndOutputAttribute();

            ImNodes::EndNode();
        }

        for (const Link& link : editor.links)
        {
            ImNodes::Link(link.id, link.start_attr, link.end_attr);
        }

        ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
        ImNodes::EndNodeEditor();

        int start_attr = -1;
        int end_attr = -1;
        if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
        {
            editor.TryCreateLink(start_attr, end_attr);
        }

        int destroyed_link_id = -1;
        if (ImNodes::IsLinkDestroyed(&destroyed_link_id))
        {
            editor.RemoveLinkById(destroyed_link_id);
        }

        if (ImGui::Button("Delete Selected Links"))
        {
            const int selected_link_count = ImNodes::NumSelectedLinks();
            if (selected_link_count > 0)
            {
                std::vector<int> selected_links(static_cast<size_t>(selected_link_count));
                ImNodes::GetSelectedLinks(selected_links.data());
                for (const int link_id : selected_links)
                {
                    editor.RemoveLinkById(link_id);
                }
                ImNodes::ClearLinkSelection();
            }
        }

        ImGui::TextUnformatted("Tip: drag from Out to In to create a link.");
        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
        glClearColor(0.09f, 0.10f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
