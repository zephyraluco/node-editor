#pragma once

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

class EditorState
{
public:
    std::vector<Node> nodes;
    std::vector<Link> links;
    
    EditorState();
    
    void AddNode();
    bool IsInputAttribute(const int attr_id) const;
    bool IsOutputAttribute(const int attr_id) const;
    void TryCreateLink(int a, int b);
    void RemoveLinkById(const int link_id);
    void Draw();

private:
    int next_node_id;
    int next_attr_id;
    int next_link_id;
};
