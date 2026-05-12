#include <node_view.h>
#include "imnodes.h"
#include "imgui.h"
#include <algorithm>
#include <cstring>

EditorState::EditorState() : next_node_id(1), next_attr_id(100), next_link_id(1000)
{
}

void EditorState::AddNode()
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

bool EditorState::IsInputAttribute(const int attr_id) const
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

bool EditorState::IsOutputAttribute(const int attr_id) const
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

void EditorState::TryCreateLink(int a, int b)
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

void EditorState::RemoveLinkById(const int link_id)
{
    links.erase(std::remove_if(links.begin(), links.end(), [link_id](const Link& link) { return link.id == link_id; }), links.end());
}

void EditorState::Draw()
{
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("Node Graph", nullptr, flags);
    if (ImGui::Button("Add Node"))
    {
        AddNode();
    }
    ImGui::SameLine();
    ImGui::Text("Nodes: %d  Links: %d", static_cast<int>(nodes.size()), static_cast<int>(links.size()));

    ImGui::BeginChild("NodeEditor", ImVec2(0, -40), true);
    ImNodes::BeginNodeEditor();

    for (Node& node : nodes)
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

    for (const Link& link : links)
    {
        ImNodes::Link(link.id, link.start_attr, link.end_attr);
    }

    ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
    ImNodes::EndNodeEditor();

    int start_attr = -1;
    int end_attr = -1;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
    {
        TryCreateLink(start_attr, end_attr);
    }

    int destroyed_link_id = -1;
    if (ImNodes::IsLinkDestroyed(&destroyed_link_id))
    {
        RemoveLinkById(destroyed_link_id);
    }
    ImGui::EndChild();
    if (ImGui::Button("Delete Selected Links"))
    {
        const int selected_link_count = ImNodes::NumSelectedLinks();
        if (selected_link_count > 0)
        {
            std::vector<int> selected_links(static_cast<size_t>(selected_link_count));
            ImNodes::GetSelectedLinks(selected_links.data());
            for (const int link_id : selected_links)
            {
                RemoveLinkById(link_id);
            }
            ImNodes::ClearLinkSelection();
        }
    }

    ImGui::TextUnformatted("Tip: drag from Out to In to create a link.");
    ImGui::End();
}
