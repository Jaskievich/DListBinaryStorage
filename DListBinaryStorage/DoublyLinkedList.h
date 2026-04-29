#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

struct ListNode {
    ListNode* prev = nullptr;

    ListNode* next = nullptr;

    ListNode* rand = nullptr;

    std::string data;
};

class DoublyLinkedList
{
public:

    ~DoublyLinkedList();
 
    void VisitNodes(std::function<void(const std::string& data, int randIndex)> visitor) const;
   
    void Add(std::vector<std::pair<std::string, int>>& entries);
 
    void DeleteList();
  
    ListNode* GetHead() const;

    size_t GetSizeList() const;
  
private:

    ListNode* head = nullptr;

    size_t count = 0;
};

class ListSerializer {
private:

    void WriteString(std::ofstream& out, const std::string& str);

    std::string ReadString(std::ifstream& in);

public:

    bool Serialize(DoublyLinkedList& linkedList, const std::string& filename);

    bool Deserialize(DoublyLinkedList& linkedList, const std::string& filename);
};