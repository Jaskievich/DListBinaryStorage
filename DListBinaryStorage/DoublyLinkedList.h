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

    DoublyLinkedList() = default;

    ~DoublyLinkedList();
    // Запрет копирования
    DoublyLinkedList(const DoublyLinkedList&) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

    // Перемещение
    DoublyLinkedList(DoublyLinkedList&& other) noexcept
        : head(std::exchange(other.head, nullptr))
        , count(std::exchange(other.count, 0)) {
    }

    DoublyLinkedList& operator=(DoublyLinkedList&& other) noexcept {
        if (this != &other) {
            DeleteList();
            head = std::exchange(other.head, nullptr);
            count = std::exchange(other.count, 0);
        }
        return *this;
    }
 
    void VisitNodes(std::function<void(const std::string& data, int randIndex)> &&visitor) const;
   
    void Add(std::vector<std::pair<std::string, int>>& entries);
 
    void DeleteList();
  
    const ListNode* GetHead() const;

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

    bool Serialize(const DoublyLinkedList& linkedList, const std::string& filename);

    bool Deserialize(DoublyLinkedList& linkedList, const std::string& filename);
};