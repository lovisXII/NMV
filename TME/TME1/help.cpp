#include <iostream>

using namespace std;

int main()
{
    uint64_t pml4_index = 0;
    uint64_t pml3_index = 1;
    uint64_t pml2_index = 0;
    uint64_t pml1_index = 0;

    uint64_t address = (pml4_index << 39) | (pml3_index << 30)| (pml2_index << 21) | (pml1_index << 12);
    cout << std::hex <<address << endl ;
}