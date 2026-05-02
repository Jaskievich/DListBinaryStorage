#include <unordered_map>
#include <fstream>
#include <cstdint>
#include <iostream>
#include "DoublyLinkedList.h"


DoublyLinkedList::~DoublyLinkedList()
{
    DeleteList();
}


void DoublyLinkedList::VisitNodes(std::function<void(const std::string& data, int randIndex)> &&visitor) const
{
    std::unordered_map<ListNode*, int> node_to_index;
    node_to_index.reserve(count);
    ListNode* current = head;
    int index = 0;
    while (current) {
      //  node_to_index[current] = index++;
        node_to_index.emplace(current, index);
        current = current->next;
    }

    current = head;
    while (current) {
        int rand_index = -1;
        if (current->rand) {
            auto it = node_to_index.find(current->rand);
            if (it != node_to_index.end()) {
                rand_index = it->second;
            }
        }
        visitor(current->data, rand_index);
        current = current->next;
    }
}

void DoublyLinkedList::Add(std::vector<std::pair<std::string, int>>& entries)
{
    if (entries.size() == 0) return;
    DeleteList();
    const size_t n = entries.size();
    std::vector<ListNode*> nodes(n);
    try {
        for (size_t i = 0; i < n; ++i) {
            nodes[i] = new ListNode();
            nodes[i]->data = std::move(entries[i].first);
        }
        // второй проход для установления rand
        for (size_t i = 0; i < n; ++i) {
            if (i > 0) {
                nodes[i]->prev = nodes[i - 1];
                nodes[i - 1]->next = nodes[i];
            }
            int rand_idx = entries[i].second;
            if (rand_idx >= 0 && rand_idx < static_cast<int>(n)) {
                nodes[i]->rand = nodes[rand_idx];
            }
        }
        head = nodes[0];
        count = n;
    }
    catch (...) {
        for (auto node : nodes) delete node;
        head = nullptr;
        count = 0;
        throw;
    }
}

//void DoublyLinkedList::Add(std::vector<std::pair<std::string, int>>& entries)
//{
//    if (entries.size() == 0) return;
//    DeleteList();
//    const size_t n = entries.size();
//    std::vector<std::unique_ptr<ListNode>> nodes(n);
//    for (size_t i = 0; i < n; ++i) {
//        nodes[i] = std::make_unique<ListNode>();
//        nodes[i]->next = nodes[i]->prev = nullptr;
//        nodes[i]->data = std::move(entries[i].first);
//    }
//    // второй проход для установления rand
//    for (size_t i = 0; i < n; ++i) {
//        if (i > 0) {
//            nodes[i]->prev = nodes[i - 1].get();
//            nodes[i - 1]->next = nodes[i].get();
//        }
//        int rand_idx = entries[i].second;
//        if (rand_idx >= 0 && rand_idx < static_cast<int>(n)) {
//            nodes[i]->rand = nodes[rand_idx].get();
//        }
//    }
//    head = nodes[0].release();
//    // Остальные unique_ptr должны быть освобождены без delete
//    for (size_t i = 1; i < n ; ++i) {
//        nodes[i].release(); // просто теряем владение
//    }
//    count = n;
//}

void DoublyLinkedList::DeleteList()
{
   
    while (head) {
        ListNode* next = head->next;
        delete head;
        head = next;
    }
    head = nullptr;
    count = 0;
}

const ListNode* DoublyLinkedList::GetHead() const {
    return head;
}

size_t DoublyLinkedList::GetSizeList() const
{
    return count;
}

// ---- ListSerializer

    // Запись строки в бинарный файл
    void ListSerializer::WriteString(std::ofstream& out, const std::string& str) {
        uint32_t len = static_cast<uint32_t>(str.size());
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        if (len > 0) {
            out.write(str.data(), len);
        }
    }

    // Чтение строки из бинарного файла
    std::string ListSerializer::ReadString(std::ifstream& in) {
        uint32_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (!in) {
            throw std::runtime_error("Ошибка чтения длины строки");
        }

        if (len > 1000 * 1000) { // защита от некорректных данных (>1MB)
            throw std::runtime_error("Строка слишком большая");
        }

        std::string str(len, '\0');
        if (len > 0) {
            in.read(&str[0], len);
            if (!in) {
                throw std::runtime_error("Ошибка чтения строки");
            }
        }
        return str;
    }



    bool ListSerializer::Serialize(const DoublyLinkedList& linkedList, const std::string& filename) {
        std::ofstream out(filename, std::ios::binary);
        if (!out) {
            std::cerr << "Ошибка: Не могу открыть файл для записи: " << filename << std::endl;
            return false;
        }
        try {

            uint32_t count32 = static_cast<uint32_t>(linkedList.GetSizeList());
            out.write(reinterpret_cast<const char*>(&count32), sizeof(count32));
            linkedList.VisitNodes([&](const std::string& data, int randIndex) {
                WriteString(out, data);
                int32_t idx = static_cast<int32_t>(randIndex);
                out.write(reinterpret_cast<const char*>(&idx), sizeof(idx));
                });
            out.close();
            std::cout << "Сериализовано " << linkedList.GetSizeList() << " узлов в " << filename << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка сериализации: " << e.what() << std::endl;
            out.close();
            return false;
        }
    }

    // Десериализация списка из бинарного файла
    bool ListSerializer::Deserialize(DoublyLinkedList& linkedList, const std::string& filename) {
        std::ifstream in(filename, std::ios::binary);
        if (!in) {
            std::cerr << "Ошибка: Не могу открыть файл для чтения: " << filename << std::endl;
            return false;
        }

        try {
            // Чтение количества узлов
            uint32_t count;
            in.read(reinterpret_cast<char*>(&count), sizeof(count));
            if (!in) {
                throw std::runtime_error("Количестов узлов не прочитано");
            }
            if (count == 0) {
                std::cout << "Десериализован пустой срисок из " << filename << std::endl;
                return true;
            }
            if (count > 1000000) { // ограничение из условия задачи
                throw std::runtime_error(" количестов узлов > 1,000,000");
            }
            std::vector<std::pair<std::string, int>> entries;
            entries.reserve(count);
            for (uint32_t i = 0; i < count; ++i) {
                std::string data = std::move(ReadString(in));
                int32_t rand_idx = -1;
                in.read(reinterpret_cast<char*>(&rand_idx), sizeof(int32_t));
                if (!in) {
                    throw std::runtime_error("Ошибка чтения rand_index " + std::to_string(i));
                }
                entries.emplace_back(data, rand_idx);

            }
            in.close();
            linkedList.Add(entries);

        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка десериализации: " << e.what() << std::endl;
            in.close();
            return false;
        }
    }
