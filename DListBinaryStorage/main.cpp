#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <functional>
#include <locale>

struct ListNode {
    ListNode* prev = nullptr;
    ListNode* next = nullptr;
    ListNode* rand = nullptr;
    std::string data;
};

class DoublyLinkedList 
{
public:

    ~DoublyLinkedList() 
    {
        DeleteList();
    }
  
   
    void VisitNodes(std::function<void(const std::string& data, int randIndex)> visitor) const 
    {
        std::unordered_map<ListNode*, int> node_to_index;
        ListNode* current = head;
        int index = 0;
        while (current) {
            node_to_index[current] = index++;
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

    void Add(std::vector<std::pair<std::string, int>>& entries)
    {
        if (entries.size() == 0) return;
        DeleteList();
        const size_t n = entries.size();
        std::vector<ListNode*> nodes(n);
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

    void DeleteList() 
    {
        while (head) {
            ListNode* next = head->next;
            delete head;
            head = next;
        }
        head = nullptr;
        count = 0;
    }

    ListNode* GetHead() const {
        return head;
    }

    size_t GetSizeList() const
    {
        return count;
    }

  
private:
    ListNode* head = nullptr;
    size_t count = 0;
};

// для прверки
//void PrintList( ListNode* head)
//{
//    ListNode* current = head;
//    int idx = 0;
//    while (current) {
//        std::cout << "Node " << idx << ": data=\"" << current->data
//            << "\", prev=" << (current->prev ? "yes" : "null")
//            << ", next=" << (current->next ? "yes" : "null")
//            << ", rand=";
//
//        if (current->rand) {
//            // Ищем индекс rand узла
//            ListNode* temp = head;
//            int rand_idx = 0;
//            while (temp && temp != current->rand) {
//                temp = temp->next;
//                rand_idx++;
//            }
//            if( temp )
//                std::cout << rand_idx <<" - "<< temp->data;
//            else std::cout << rand_idx ;
//        }
//        else {
//            std::cout << "null";
//        }
//        std::cout << std::endl;
//
//        current = current->next;
//        idx++;
//    }
//}

void PrintList(DoublyLinkedList  & linkedList)
{
    int index = 0;
    linkedList.VisitNodes([&](const std::string& data, int randIndex) {

        std::cout << "Node " << index << ": data = \"" << data << "\",rand_idx = " << randIndex << std::endl;
        index++;
     });

}


static void BuildFromTextFile(const std::string& filename, DoublyLinkedList &list) 
{
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Ошибка открытия файла " << filename << std::endl;
        return ;
    }
    std::vector<std::pair<std::string, int>> entries;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        size_t semicolon = line.rfind(';');
        if (semicolon == std::string::npos) {
            std::cerr << "Неверный формат" << line << std::endl;
            continue;
        }
        std::string data = line.substr(0, semicolon);
        int rand_idx = std::stoi(line.substr(semicolon + 1));
        entries.emplace_back( data, rand_idx );
    }
    in.close();

    if (entries.empty()) {
        return ;
    }

    list.Add(entries);
}

class ListSerializer {
private:

    // Запись строки в бинарный файл
    void WriteString(std::ofstream& out, const std::string& str) {
        uint32_t len = static_cast<uint32_t>(str.size());
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        if (len > 0) {
            out.write(str.data(), len);
        }
    }

    // Чтение строки из бинарного файла
    std::string ReadString(std::ifstream& in) {
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


public:


    bool Serialize(DoublyLinkedList& linkedList, const std::string& filename) {
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
            std::cout << "Сериализовано " << linkedList.GetSizeList()  << " узлов в " << filename << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка сериализации: " << e.what() << std::endl;
            out.close();
            return false;
        }
    }

    // Десериализация списка из бинарного файла
    bool Deserialize(DoublyLinkedList& linkedList, const std::string& filename) {
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
                entries.emplace_back( data, rand_idx );
                
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
};

int main() {

    setlocale(LC_ALL, "Russian");
    const std::string input_file = "inlet.in";
    const std::string output_file = "outlet.out";

    std::cout << "=== Сериализация двусвязного списка ===" << std::endl;
    DoublyLinkedList linkedList;
    BuildFromTextFile(input_file, linkedList);
  //  PrintList(linkedList.GetHead());
    PrintList(linkedList);

    ListSerializer serializer;

    serializer.Serialize(linkedList, output_file);

    std::cout << "=== Удаление списка и проверка ===" << std::endl;
    linkedList.DeleteList();
 //   PrintList(linkedList.GetHead());
    PrintList(linkedList);
    std::cout << "=== Проверка сериализации двусвязного списка ===" << std::endl;
    serializer.Deserialize(linkedList, output_file);
   // PrintList(linkedList.GetHead());
    PrintList(linkedList);

    linkedList.DeleteList();
    

    return 0;
}