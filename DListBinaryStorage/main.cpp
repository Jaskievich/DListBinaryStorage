#include <iostream>
#include <fstream>
#include <vector>

#include <string>
#include <cstdint>

#include <locale>

#include "DoublyLinkedList.h"

static void PrintList(DoublyLinkedList  & linkedList)
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