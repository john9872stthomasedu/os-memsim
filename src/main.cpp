#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include "mmu.h"
#include "pagetable.h"
#include <sstream>

// 64 MB (64 * 1024 * 1024)
#define PHYSICAL_MEMORY 67108864

void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, uint8_t *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table);

int main(int argc, char **argv)
{
    // Ensure user specified page size as a command line parameter
    if (argc < 2)
    {
        std::cerr << "Error: you must specify the page size" << std::endl;
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory' (raw array of bytes)
    uint8_t *memory = new uint8_t[PHYSICAL_MEMORY];

    // Create MMU and Page Table
    Mmu *mmu = new Mmu(PHYSICAL_MEMORY);
    PageTable *page_table = new PageTable(page_size);

    // Prompt loop
    std::string command;
    std::cout << "> ";
    std::getline(std::cin, command);
    while (command != "exit")
    {
        // Handle command
        // TODO: implement this!
        std::vector<std::string> args;
        std::stringstream ss(command);
        std::string token;
        while(ss >> token){
            args.push_back(token);
        }

        if(args[0] == "create"){
            createProcess(std::stoi(args[1]), std::stoi(args[2]), mmu, page_table);
        }

        else if(args[0] == "allocate"){
            //allocateVariable(std::stoi(args[1]), args[2], args[3], std::stoi(args[4]), mmu, page_table);
            DataType type;
            if(args[3] == "char") type = DataType::Char;
            else if(args[3] == "short") type = DataType::Short;
            else if(args[3] == "int") type = DataType::Int;
            else if(args[3] == "float") type = DataType::Float;
            else if(args[3] == "long") type = DataType::Long;
            else if(args[3] == "double") type = DataType::Double;

            allocateVariable(std::stoi(args[1]), args[2], type,
                            std::stoi(args[4]), mmu, page_table);
        }

        else if(args[0] == "set"){
        uint32_t pid = std::stoi(args[1]);
        std::string var = args[2];
        uint32_t offset = std::stoi(args[3]);

        Variable* varInfo = mmu->getVariable(pid, var);

        if (!varInfo) {
            std::cout << "error: variable not found" << std::endl;
            continue;
        }

        
        if (varInfo->type == DataType::Char)
        {
            // handle characters
            for (int i = 4; i < args.size(); i++)
            {
                char value = args[i][0];  // take first char
                setVariable(pid, var, offset + (i - 4),
                            &value, mmu, page_table, memory);
            }
        }
        else
        {
            // handle numeric types
            for (int i = 4; i < args.size(); i++)
            {
                try {
                    int value = std::stoi(args[i]);
                    setVariable(pid, var, offset + (i - 4),
                                &value, mmu, page_table, memory);
                }
                catch (...) {
                    std::cout << "error: invalid integer input" << std::endl;
                    break;
                }
            }
        }
    }

        else if(args[0] == "free"){
            freeVariable(std::stoi(args[1]), args[2], mmu, page_table);
        }

        else if(args[0] == "terminate"){
            terminateProcess(std::stoi(args[1]), mmu, page_table);
        }

        else if(args[0] == "print"){
            if(args[1] =="mmu"){
                mmu->print();
            }
            else if(args[1] =="page"){
                page_table->print();
            }
            else if(args[1] =="processes"){
                mmu->printProcesses();
            }
            else{
                std::string input = args[1];
                size_t pos = input.find(":");

                if (pos != std::string::npos) {
                    uint32_t pid = std::stoi(input.substr(0, pos));
                    std::string var_name = input.substr(pos + 1);

                    Process* proc = mmu->getProcess(pid);
                    if (!proc) {
                        std::cout << "error: process not found" << std::endl;
                        continue;
                    }

                    Variable* var = mmu->getVariable(pid, var_name);
                    if (!var) {
                        std::cout << "error: variable not found" << std::endl;
                        continue;
                    }

                    if (var->type == DataType::Char)
                    {
                        uint32_t count = var->size;

                        uint32_t limit = std::min(count, (uint32_t)16);

                        for (uint32_t i = 0; i < limit; i++) {
                            int phys = page_table->getPhysicalAddress(pid, var->virtual_address + i);
                            char value;
                            memcpy(&value, &memory[phys], sizeof(char));

                            std::cout << value;
                            if (i < limit - 1) std::cout << " ";
                        }

                        if (count > limit) {
                            std::cout << " ... [" << count << " chars]";
                        }

                        std::cout << std::endl;
                    }
                    else
                    {
                        uint32_t count = var->size / 4;

                        uint32_t limit = std::min(count, (uint32_t)4);

                        for (uint32_t i = 0; i < limit; i++) {
                            int phys = page_table->getPhysicalAddress(pid, var->virtual_address + i * 4);
                            int value;
                            memcpy(&value, &memory[phys], sizeof(int));

                            std::cout << value;
                            if (i < limit - 1) std::cout << ", ";
                        }

                        if (count > 4) {
                            std::cout << ", ... [" << count << " items]";
                        }

                        std::cout << std::endl;
                    }


                    std::cout << std::endl;
                }   
            }
        }

        else{
            std::cout << "error: command not recognized" << std::endl;
        }

        // Get next command
        std::cout << "> ";
        std::getline(std::cin, command);
    }

    // Cean up
    delete[] memory;
    delete mmu;
    delete page_table;

    return 0;
}

void printStartMessage(int page_size)
{
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std:: endl;
    std::cout << "Commands:" << std:: endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std:: endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std:: endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std:: endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std:: endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std:: endl;
    std::cout << "  * print <object> (prints data)" << std:: endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std:: endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std:: endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std:: endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std:: endl;
    std::cout << std::endl;
}

void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - create new process in the MMU
    //   - allocate new variables for the <TEXT>, <GLOBALS>, and <STACK>
    //   - print pid
    uint32_t pid = mmu->createProcess();
    mmu->addVariableToProcess(pid, "<TEXT>", DataType::Int, text_size, 0);
    mmu->addVariableToProcess(pid, "<GLOBALS>", DataType::Int, data_size, text_size);
    mmu->addVariableToProcess(pid, "<STACK>", DataType::Int, 65536, text_size+data_size);
    page_table->addEntry(pid, 0);
    for(int i = 1; i*page_table->getPageSize() < text_size+data_size+65536; i = i + 1){
        page_table->addEntry(pid, i);
    }
    std::cout << pid << std::endl;
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - find first free space within a page already allocated to this process that is large enough to fit the new variable
    //   - if no hole is large enough, allocate new page(s)
    //   - insert variable into MMU
    //   - print virtual memory address


    Process* proc = mmu->getProcess(pid);
        //testing
    std::cout << "DEBUG FREE LIST:\n";
for (auto v : proc->variables)
{
    std::cout
        << v->name
        << " type=" << (int)v->type
        << " addr=" << v->virtual_address
        << " size=" << v->size
        << "\n";
}


    //end of testing
    if (!proc) {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    if (mmu->getVariable(pid, var_name)) {
        std::cout << "error: variable already exists" << std::endl;
        return;
    }

    uint32_t type_size;
    switch(type){
        case DataType::Char: type_size = 1; break;
        case DataType::Short: type_size = 2; break;
        case DataType::Int:
        case DataType::Float: type_size = 4; break;
        case DataType::Long:
        case DataType::Double: type_size = 8; break;
        default: return;
    }

    uint32_t total_size = type_size * num_elements;
    uint32_t base = 0;

    for (auto v : proc->variables)
    {
        if (v->name == "<TEXT>" ||
            v->name == "<GLOBALS>" ||
            v->name == "<STACK>")
            continue;
    
            uint32_t end = v->virtual_address + v->size;
            if (end > base)
                base = end;
        
    }

    uint32_t total_used = 0;

    for (auto v : proc->variables) {
        if (v->type != DataType::FreeSpace)
            total_used += v->size;
    }

    if (total_used + total_size > PHYSICAL_MEMORY) {
        std::cout << "error: allocation exceeds system memory" << std::endl;
        return;
    }

   uint32_t addr = 0;

// always start AFTER reserved regions
for (auto v : proc->variables)
{
    if (v->name == "<TEXT>" ||
        v->name == "<GLOBALS>" ||
        v->name == "<STACK>")
    {
        uint32_t end = v->virtual_address + v->size;
        if (end > addr)
            addr = end;
    }
}

// now place after heap start
for (auto v : proc->variables)
{
    if (v->name != "<TEXT>" &&
        v->name != "<GLOBALS>" &&
        v->name != "<STACK>" &&
        v->name != "<FREE_SPACE>")
    {
        uint32_t end = v->virtual_address + v->size;
        if (end > addr)
            addr = end;
    }
}

    mmu->addVariableToProcess(pid, var_name, type, total_size, addr);

    uint32_t start_page = addr / page_table->getPageSize();
    uint32_t end_page = (addr + total_size) / page_table->getPageSize();

    for(uint32_t p = start_page; p <= end_page; p++){
        page_table->addEntry(pid, p);
    }

    std::cout << addr << std::endl;


}

void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, uint8_t *memory)
{
    // TODO: implement this!
    //   - look up physical address for variable based on its virtual address / offset
    //   - insert `value` into `memory` at physical address
    //   * note: this function only handles a single element (i.e. you'll need to call this within a loop when setting
    //           multiple elements of an array)
    Process* proc = mmu->getProcess(pid);
    if (!proc) {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    Variable* var = mmu->getVariable(pid, var_name);
    if (!var) {
        std::cout << "error: variable not found" << std::endl;
        return;
    }

    if (offset >= var->size) {
        std::cout << "error: index out of range" << std::endl;
        return;
    }

    uint32_t vaddr = var->virtual_address + offset;

    int phys = page_table->getPhysicalAddress(pid, vaddr);

    if (phys < 0) return;

    uint32_t size;

    switch(var->type){
        case DataType::Char: size = 1; break;
        case DataType::Short: size = 2; break;
        case DataType::Int:
        case DataType::Float: size = 4; break;
        case DataType::Long:
        case DataType::Double: size = 8; break;
        default: size = 4;
    }

    memcpy(&memory[phys], value, size);
}

void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove entry from MMU
    //   - free page if this variable was the only one on a given page
    if (!mmu->getProcess(pid)) {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    if (!mmu->getVariable(pid, var_name)) {
        std::cout << "error: variable not found" << std::endl;
        return;
    }

    mmu->removeVariable(pid, var_name);
    page_table->removeEntriesForProcess(pid);
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove process from MMU
    //   - free all pages associated with given process
    if (!mmu->getProcess(pid)) {
        std::cout << "error: process not found" << std::endl;
        return;
    }
    page_table->removeEntriesForProcess(pid);
    mmu->removeProcess(pid);
}
