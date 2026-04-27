#include <iostream>
#include <algorithm>
#include "mmu.h"

Mmu::Mmu(int memory_size)
{
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu()
{
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process();
    proc->pid = _next_pid;

    Variable *var = new Variable();
    var->name = "<FREE_SPACE>";
    var->type = DataType::FreeSpace;
    var->virtual_address = 0;
    var->size = _max_size;
    proc->variables.push_back(var);

    _processes.push_back(proc);

    _next_pid++;
    
    return proc->pid;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address)
{
    int i;
    Process *proc = NULL;
    std::vector<Process*>::iterator it = std::find_if(_processes.begin(), _processes.end(), [pid](Process* p)
    { 
        return p != nullptr && p->pid == pid; 
    });

    if (it != _processes.end())
    {
        Process *proc = *it;
        Variable *var = new Variable();
        var->name = var_name;
        var->type = type;
        var->virtual_address = address;
        var->size = size;
        proc->variables.push_back(var);
    }
}

void Mmu::print()
{
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++)
    {
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            // TODO: print all variables (excluding those of type DataType::FreeSpace)
            if(_processes[i]->variables[j]->type != DataType::FreeSpace){
                printf(" %d | %-13s | 0x%08X | %10d\n",
                _processes[i]->pid,
                _processes[i]->variables[j]->name.c_str(),
                _processes[i]->variables[j]->virtual_address,
                _processes[i]->variables[j]->size
            );   
            }
        }
    }
}

void Mmu::printProcesses(){
    for(int i = 0; i < _processes.size(); i++){
        std::cout << _processes[i]->pid << std::endl;
    }
}

Process* Mmu::getProcess(uint32_t pid)
{
    for (auto p : _processes)
    {
        if (p->pid == pid)
            return p;
    }
    return nullptr;
}

Variable* Mmu::getVariable(uint32_t pid, std::string name)
{
    Process* p = getProcess(pid);
    if (!p) return nullptr;

    for (auto v : p->variables)
    {
        if (v->name == name)
            return v;
    }
    return nullptr;
}

void Mmu::removeVariable(uint32_t pid, std::string name)
{
    Process* p = getProcess(pid);
    if (!p) return;

    for (int i = 0; i < p->variables.size(); i++)
    {
        if (p->variables[i]->name == name)
        {
            delete p->variables[i];
            p->variables.erase(p->variables.begin() + i);
            return;
        }
    }
}

void Mmu::removeProcess(uint32_t pid)
{
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            for (auto v : _processes[i]->variables)
                delete v;

            delete _processes[i];
            _processes.erase(_processes.begin() + i);
            return;
        }
    }
}
