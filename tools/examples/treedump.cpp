// Just show some position data

#include <iostream>
#include <iomanip>
#include <climits>
#include <vector>
#include <sstream>
#include <ctime>
#include <cstdio>
using namespace std;

#define DFHACK_WANT_MISCUTILS
#include <DFHack.h>

int main (int numargs, const char ** args)
{
    uint32_t addr;
    if (numargs == 2)
    {
        istringstream input (args[1],istringstream::in);
        input >> std::hex >> addr;
    }
    DFHack::ContextManager DFMgr("Memory.xml");
    DFHack::Context * DF;
    try
    {
        DF = DFMgr.getSingleContext();
        DF->Attach();
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
        #ifndef LINUX_BUILD
            cin.ignore();
        #endif
        return 1;
    }

    DFHack::Process* p = DF->getProcess();
    DFHack::memory_info* mem = DF->getMemoryInfo();
    DFHack::Position * pos = DF->getPosition();
    DFHack::Vegetation * v = DF->getVegetation();
    DFHack::Materials * mat = DF->getMaterials();
    mat->ReadOrganicMaterials();

    int32_t x,y,z;
    pos->getCursorCoords(x,y,z);

    uint32_t numVegs = 0;
    v->Start(numVegs);
    if(x == -30000)
    {
        cout << "----==== Trees ====----" << endl;
        for(uint32_t i =0; i < numVegs; i++)
        {
            DFHack::t_tree tree;
            v->Read(i,tree);
            printf("%d/%d/%d, %d:%d\n",tree.x,tree.y,tree.z,tree.type,tree.material);
        }
    }
    else
    {
        cout << "----==== Tree at "<< x << "/" << y << "/" << z << " ====----" << endl;
        for(uint32_t i =0; i < numVegs; i++)
        {
            DFHack::t_tree tree;
            v->Read(i,tree);
            if(tree.x == x && tree.y == y && tree.z == z)
            {
                printf("%d:%d = ",tree.type,tree.material);
                if(tree.type == 1 || tree.type == 3)
                {
                    cout << "near-water ";
                }
                cout << mat->organic[tree.material].id << " ";
                if(tree.type == 0 || tree.type == 1)
                {
                    cout << "tree";
                }
                if(tree.type == 2 || tree.type == 3)
                {
                    cout << "shrub";
                }
                cout << endl;
                printf("Address: 0x%x\n", tree.address);
                hexdump(DF,tree.address,13);
                break;
            }
        }
    }
    v->Finish();

    #ifndef LINUX_BUILD
    cout << "Done. Press any key to continue" << endl;
    cin.ignore();
    #endif
    return 0;
}
