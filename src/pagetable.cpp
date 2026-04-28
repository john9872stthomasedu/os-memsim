#include <algorithm>
#include "pagetable.h"

PageTable::PageTable(int page_size)
{
    _page_size = page_size;
}

PageTable::~PageTable()
{
}

std::vector<std::string> PageTable::sortedKeys()
{
    std::vector<std::string> keys;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end(), PageTableKeyComparator());

    return keys;
}

void PageTable::addEntry(uint32_t pid, int page_number)
{
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    int frame = 0; 
    // Find free frame
    // TODO: implement this!
    std::vector<std::string> keys = sortedKeys();
    frame = keys.size();
    for(int i = 0; i < keys.size(); i++){
        bool found = false;
        for(int j = 0; j < keys.size(); j++){
            if(_table[keys[j]] == i){
                found = true;
            }
        }
        if(!found){
            frame = i;
            break;
        }
    }
    _table[entry] = frame;
}

int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address)
{
    // Convert virtual address to page_number and page_offset
    // TODO: implement this!
    int page_number = virtual_address / _page_size;
    int page_offset = virtual_address % _page_size;

    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    
    // If entry exists, look up frame number and convert virtual to physical address
    int address = -1;
    if (_table.count(entry) > 0)
    {
        // TODO: implement this!
        int frame = _table[entry];
        return frame * _page_size + page_offset;
    }

    return address;
}

int PageTable::getPageSize(){
    return _page_size;
}

void PageTable::print()
{
    int i;

    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    std::vector<std::string> keys = sortedKeys();

    for (i = 0; i < keys.size(); i++)
    {
        // TODO: print all pages
        std::string key = keys[i];
        uint32_t pid = std::stoi(key.substr(0, key.find("|")));
        uint32_t page = std::stoi(key.substr(key.find("|")+1));

        printf(" %d | %11d | %12d\n", pid, page, _table[key]);
    }
}

void PageTable::removeEntriesForProcess(uint32_t pid)
{
    std::vector<std::string> keys = sortedKeys();

    for (auto key : keys)
    {
        // key format: "pid|page"
        if (key.find(std::to_string(pid) + "|") == 0)
        {
            _table.erase(key);
        }
    }
}

void PageTable::removeEntry(uint32_t pid, uint32_t page_number)
{
    //testing
    std::cout << "trying remove: " 
          << std::to_string(pid) + "|" + std::to_string(page_number)
          << std::endl;
    //end of testing
    std::string key = std::to_string(pid) + "|" + std::to_string(page_number);

    auto it = _table.find(key);
    if (it != _table.end())
    {
        _table.erase(it);
    }
}