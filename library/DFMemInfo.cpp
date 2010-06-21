/*
www.sourceforge.net/projects/dfhack
Copyright (c) 2009 Petr Mrázek (peterix), Kenneth Ferland (Impaler[WrG]), dorf

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#include "Internal.h"
#include "dfhack/DFMemInfo.h"
#include "dfhack/DFError.h"
#include "dfhack/DFProcess.h"

//Inital amount of space in levels vector (since we usually know the number, efficent!)
#define NUM_RESERVE_LVLS 20
#define NUM_RESERVE_MOODS 6
using namespace DFHack;

/*
* Common data types
*/
namespace DFHack
{
    struct t_type
    {
        t_type(uint32_t assign, uint32_t type, std::string classname)
        :classname(classname),assign(assign),type(type){};
        std::string classname;
        uint32_t assign;
        uint32_t type;
    };

    struct t_class
    {
        t_class(const t_class &old)
        {
            classname = old.classname;
            vtable = old.vtable;
            assign = old.assign;
            type_offset = old.type_offset;
            for(uint32_t i = 0; i < old.subs.size();i++)
            {
                t_type * t = new t_type (*old.subs[i]);
                subs.push_back(t);
            }
        }
        t_class ()
        {
            vtable = 0;
            assign = 0;
            type_offset = 0;
        }
        ~t_class()
        {
            for(uint32_t i = 0; i < subs.size();i++)
            {
                delete subs[i];
            }
            subs.clear();
        }
        std::string classname;
        uint32_t vtable;
        uint32_t assign;// index to typeclass array if multiclass. return value if not.
        uint32_t type_offset; // offset of type data for multiclass
        std::vector<t_type *> subs;
    };
}
/*
 * Private data
 */
class memory_info::Private
{
    public:
    map <string, uint32_t> addresses;
    map <string, int32_t> offsets;
    map <string, uint32_t> hexvals;
    map <string, string> strings;
    
    vector<string> professions;
    vector<string> jobs;
    vector<string> skills;
	vector<DFHack::t_level> levels;
    vector< vector<string> > traits;
	vector<string> moods;
    map <uint32_t, string> labors;
    
    // storage for class and multiclass
    vector<t_class *> classes;
    
    // cache for faster name lookup, indexed by classID
    vector<string> classnames;
    // map between vptr and class id, needs further type id lookup for multi-classes, not inherited
    map<uint32_t, t_class *> classIDs;
    
    // index for the next added class
    uint32_t classindex;
    
    int32_t base;
    Process * p; // the process this belongs to
    
    string version;
    OSType OS;
};

// normal constructor
memory_info::memory_info()
:d(new Private)
{
    d->base = 0;
    d->p = 0;
    d->classindex = 0;
	d->levels.reserve(NUM_RESERVE_LVLS);
	d->moods.reserve(NUM_RESERVE_MOODS);
}

// copy constructor
memory_info::memory_info(const memory_info &old)
:d(new Private)
{
    d->version = old.d->version;
    d->OS = old.d->OS;
    d->addresses = old.d->addresses;
    d->offsets = old.d->offsets;
    d->hexvals = old.d->hexvals;
    d->strings = old.d->strings;
    d->base = old.d->base;
    //d->classes = old.d->classes;
    for(uint32_t i = 0; i < old.d->classes.size(); i++)
    {
        t_class * copy = new t_class(*old.d->classes[i]);
        d->classes.push_back(copy);
    }
    d->classnames = old.d->classnames;
    d->classindex = old.d->classindex;
    d->professions = old.d->professions;
    d->jobs = old.d->jobs;
    d->skills = old.d->skills;
    d->traits = old.d->traits;
    d->labors = old.d->labors;
	d->levels = old.d->levels;
	d->moods = old.d->moods;
}
void memory_info::setParentProcess(Process * _p)
{
    d->p = _p;
}

// destructor
memory_info::~memory_info()
{
    // delete the vtables
    for(uint32_t i = 0; i < d->classes.size();i++)
    {
        delete d->classes[i];
    }
    // delete our data
    delete d;
}

void memory_info::setVersion(const char * v)
{
    d->version = v;
}


void memory_info::setVersion(const string &v)
{
    d->version = v;
}


string memory_info::getVersion()
{
    return d->version;
}


void memory_info::setOS(const char *os)
{
    string oss = os;
    if(oss == "windows")
        d->OS = OS_WINDOWS;
    else if(oss == "linux")
        d->OS = OS_LINUX;
    else
        d->OS = OS_BAD;
}


void memory_info::setOS(const string &os)
{
    if(os == "windows")
        d->OS = OS_WINDOWS;
    else if(os == "linux")
        d->OS = OS_LINUX;
    else
        d->OS = OS_BAD;
}


void memory_info::setOS(OSType os)
{
    if(os >= OS_WINDOWS && os < OS_BAD)
    {
        d->OS = os;
        return;
    }
    d->OS = OS_BAD;
}


memory_info::OSType memory_info::getOS() const
{
    return d->OS;
}

uint32_t memory_info::getBase () const
{
    return d->base;
}


void memory_info::setBase (const string &s)
{
    d->base = strtol(s.c_str(), NULL, 16);
}


void memory_info::setBase (const uint32_t b)
{
    d->base = b;
}


void memory_info::setOffset (const string & key, const string & value)
{
    int32_t offset = strtol(value.c_str(), NULL, 16);
    d->offsets[key] = offset;
}


void memory_info::setAddress (const string & key, const string & value)
{
    uint32_t address = strtol(value.c_str(), NULL, 16);
    d->addresses[key] = address;
}


void memory_info::setHexValue (const string & key, const string & value)
{
    uint32_t hexval = strtol(value.c_str(), NULL, 16);
    d->hexvals[key] = hexval;
}


void memory_info::setString (const string & key, const string & value)
{
    d->strings[key] = value;
}

void memory_info::setLabor(const string & key, const string & value)
{
    uint32_t keyInt = strtol(key.c_str(), NULL, 10);
    d->labors[keyInt] = value;
}

void memory_info::setProfession (const string & key, const string & value)
{
    uint32_t keyInt = strtol(key.c_str(), NULL, 10);
    if(d->professions.size() <= keyInt)
    {
        d->professions.resize(keyInt+1,"");
    }
    d->professions[keyInt] = value;
}

void memory_info::setJob (const string & key, const string & value)
{
    uint32_t keyInt = strtol(key.c_str(), NULL, 10);
    if(d->jobs.size() <= keyInt)
    {
        d->jobs.resize(keyInt+1);
    }
    d->jobs[keyInt] = value;
}

void memory_info::setSkill (const string & key, const string & value)
{
    uint32_t keyInt = strtol(key.c_str(), NULL, 10);
    if(d->skills.size() <= keyInt){
        d->skills.resize(keyInt+1);
    }
    d->skills[keyInt] = value;
}

void memory_info::setLevel(const std::string &nLevel,
						   const std::string &nName,
						   const std::string &nXp)
{
	uint32_t keyInt = strtol(nLevel.c_str(), NULL, 10);
    
	if(d->levels.size() <= keyInt)
		d->levels.resize(keyInt+1);
	
	d->levels[keyInt].level = keyInt;
	d->levels[keyInt].name = nName;
	d->levels[keyInt].xpNxtLvl = strtol(nXp.c_str(), NULL, 10);
}

void memory_info::setMood(const std::string &id, const std::string &mood)
{
	uint32_t keyInt = strtol(id.c_str(), NULL, 10);
    
	if(d->moods.size() <= keyInt)
		d->moods.resize(keyInt+1);

	d->moods[keyInt] = mood;
}

void memory_info::setTrait(const string & key,
                           const string & value,
                           const string & zero,
                           const string & one,
                           const string & two,
                           const string & three,
                           const string & four,
                           const string & five)
{
    uint32_t keyInt = strtol(key.c_str(), NULL, 10);
    if(d->traits.size() <= keyInt)
    {
        d->traits.resize(keyInt+1);
    }
    d->traits[keyInt].push_back(zero);
    d->traits[keyInt].push_back(one);
    d->traits[keyInt].push_back(two);
    d->traits[keyInt].push_back(three);
    d->traits[keyInt].push_back(four);
    d->traits[keyInt].push_back(five);
    d->traits[keyInt].push_back(value);
}

// FIXME: next three methods should use some kind of custom container so it doesn't have to search so much.
t_class * memory_info::setClass (const char * name, uint32_t vtable, uint32_t typeoffset)
{
    if(name == 0)
        return 0;
    for (uint32_t i=0; i<d->classes.size(); i++)
    {
        if(d->classes[i]->classname == name)
        {
            if(vtable != 0)
                d->classes[i]->vtable = vtable;
            if(typeoffset != 0)
                d->classes[i]->type_offset = typeoffset;
            return d->classes[i];
        }
    }
    
    t_class *cls = new t_class();
    // get an unique ID and add ourselves to the index
    cls->assign = d->classindex;
    cls->classname = name;
    d->classnames.push_back(name);
        
    // vtables no longer a requirement
    cls->vtable = vtable;
        
    // multi class yes/no
    cls->type_offset = typeoffset;
    
    d->classes.push_back(cls);
    d->classindex++;
    return cls;
    
}


void memory_info::setClassChild (t_class * parent, const char * name, const char * type)
{
    vector <t_type *>& vec = parent->subs;
    for (uint32_t i=0; i<vec.size(); i++)
    {
        if(vec[i]->classname == name)
        {
            vec[i]->type = strtol(type, NULL, 16);
            return;
        }
    }
    // new multiclass child
    t_type *mcc = new t_type(d->classindex,strtol(type, NULL, 16),name);
    d->classnames.push_back(name);
    vec.push_back(mcc);
    d->classindex++;
    
    //cout << "    classtype " << name << ", assign " << mcc->assign << ", vtable  " << mcc->type << endl;
}

// FIXME: stupid. we need a better container
bool memory_info::resolveObjectToClassID(const uint32_t address, int32_t & classid)
{
    uint32_t vtable = d->p->readDWord(address);
    // try to find the vtable in our cache
    map<uint32_t, t_class *>::iterator it;
    it = d->classIDs.find(vtable);
    t_class * cl;
    
    // class found in cache?
    if(it != d->classIDs.end())
    {
        cl = (*it).second;
    }
    else// couldn't find?
    {
        // we set up the class for the first time
        string classname = d->p->readClassName(vtable);
        d->classIDs[vtable] = cl = setClass(classname.c_str(),vtable);
    }
    // and isn't a multi-class
    if(!cl->type_offset)
    {
        // return
        classid = cl->assign;
        return true;
    }
    // and is a multiclass
    else
    {
        // find the type
        vector <t_type*>& vec = cl->subs;
        uint32_t type = d->p->readWord(address + cl->type_offset);
        // return typed building if successful
        //FIXME: the vector should map directly to the type IDs here, so we don't have to mess with O(n) search
        for (uint32_t k = 0; k < vec.size();k++)
        {
            if(vec[k]->type == type)
            {
                //cout << " multi " <<  address + classes[i].type_offset << " " << vec[k].classname << endl;
                classid = vec[k]->assign;
                return true;
            }
        }
        // couldn't find the type... now what do we do here? throw?
        // this is a case where it might be a good idea, because it uncovers some deeper problems
        // we return the parent class instead, it doesn't change the API semantics and sort-of makes sense
        classid = cl->assign;
        return true;
    }
}

//ALERT: doesn't care about multiclasses
bool memory_info::resolveClassnameToVPtr(const string classname, uint32_t & vptr)
{
    // FIXME: another stupid search.
    for(uint32_t i = 0;i< d->classes.size();i++)
    {
        //if(classes[i].)
        if(d->classes[i]->classname == classname) // got class
        {
            vptr = d->classes[i]->vtable;
            return true;
        }
    }
    // we failed to find anything that would match
    return false;
}

bool memory_info::resolveClassnameToClassID (const string classname, int32_t & classID)
{
    // FIXME: another stupid search.
    classID = -1;
    for(uint32_t i = 0;i< d->classnames.size();i++)
    {
        if(d->classnames[i] == classname)
        {
            classID = i;
            return true;
        }
    }
    // we failed to find anything that would match
    return false;
}

bool memory_info::resolveClassIDToClassname (const int32_t classID, string & classname)
{
    if (classID >=0 && (uint32_t)classID < d->classnames.size())
    {
        classname = d->classnames[classID];
        return true;
    }
    return false;
}


// return pointer to our internal classID -> className mapping
const vector<string> * memory_info::getClassIDMapping()
{
    return &d->classnames;
}


// change base of all addresses
void memory_info::RebaseAddresses(const int32_t new_base)
{
    map<string, uint32_t>::iterator iter;
    int32_t rebase = - (int32_t)d->base + new_base;
    for(iter = d->addresses.begin(); iter != d->addresses.end(); iter++)
    {
        d->addresses[iter->first] = iter->second + rebase;
    }
}


// change base of all addresses *and* vtable entries
void memory_info::RebaseAll(int32_t new_base)
{
    map<string, uint32_t>::iterator iter;
    int32_t rebase = - (int32_t)d->base + new_base;
    for(iter = d->addresses.begin(); iter != d->addresses.end(); iter++)
    {
        d->addresses[iter->first] = iter->second + rebase;
    }
    RebaseVTable(rebase);
}


// change all vtable entries by offset
void memory_info::RebaseVTable(int32_t offset)
{
    vector<t_class *>::iterator iter;
    for(iter = d->classes.begin(); iter != d->classes.end(); iter++)
    {
        (*iter)->vtable += offset;
    }
}

// Get named address
uint32_t memory_info::getAddress (const char *key)
{
    map <string, uint32_t>::iterator iter = d->addresses.find(key);
    
    if(iter != d->addresses.end())
    {
        return (*iter).second;
    }
    throw Error::MissingMemoryDefinition("address", key);
}


// Get named offset
int32_t memory_info::getOffset (const char *key)
{
    map <string, int32_t>::iterator iter = d->offsets.find(key);
    if(iter != d->offsets.end())
    {
        return (*iter).second;
    }
    throw Error::MissingMemoryDefinition("offset", key);
}

// Get named numerical value
uint32_t memory_info::getHexValue (const char *key)
{
    map <string, uint32_t>::iterator iter = d->hexvals.find(key);
    if(iter != d->hexvals.end())
    {
        return (*iter).second;
    }
    throw Error::MissingMemoryDefinition("hexvalue", key);
}


// Get named address
uint32_t memory_info::getAddress (const string &key)
{
    map <string, uint32_t>::iterator iter = d->addresses.find(key);
    
    if(iter != d->addresses.end())
    {
        return (*iter).second;
    }
    throw Error::MissingMemoryDefinition("address", key.c_str());
}


// Get named offset
int32_t memory_info::getOffset (const string &key)
{
    map <string, int32_t>::iterator iter = d->offsets.find(key);
    if(iter != d->offsets.end())
    {
        return (*iter).second;
    }
    throw Error::MissingMemoryDefinition("offset", key.c_str());
}

// Get named numerical value
uint32_t memory_info::getHexValue (const string &key)
{
    map <string, uint32_t>::iterator iter = d->hexvals.find(key);
    if(iter != d->hexvals.end())
    {
        return (*iter).second;
    }
    throw Error::MissingMemoryDefinition("hexvalue", key.c_str());
}

// Get named string
std::string memory_info::getString (const string &key)
{
    map <string, string>::iterator iter = d->strings.find(key);
    if(iter != d->strings.end())
    {
        return (*iter).second;
    }
    throw Error::MissingMemoryDefinition("string", key.c_str());
}

// Get Profession
string memory_info::getProfession (const uint32_t key) const
{
    if(d->professions.size() > key)
    {
        return d->professions[key];
    }
    throw Error::MissingMemoryDefinition("profession", key);
}

// Get Job
string memory_info::getJob (const uint32_t key) const
{
    if(d->jobs.size() > key)
    {
        return d->jobs[key];
    }
    throw Error::MissingMemoryDefinition("job", key);
}

string memory_info::getSkill (const uint32_t key) const
{
    if(d->skills.size() > key)
    {
        return d->skills[key];
    }
    throw Error::MissingMemoryDefinition("skill", key);
}

DFHack::t_level memory_info::getLevelInfo(const uint32_t level) const
{
	if(d->levels.size() > level)
    {
        return d->levels[level];
    }
    throw Error::MissingMemoryDefinition("Level", level);
}

// FIXME: ugly hack that needs to die
int absolute (int number)
{
    if (number < 0)
        return -number;
    return number;
}

string memory_info::getTrait (const uint32_t traitIdx, const uint32_t traitValue) const
{
    if(d->traits.size() > traitIdx)
    {
        int diff = absolute(traitValue-50);
        if(diff < 10)
        {
            return string("");
        }
        if (traitValue >= 91)
            return d->traits[traitIdx][5];
        else if (traitValue >= 76)
            return d->traits[traitIdx][4];
        else if (traitValue >= 61)
            return d->traits[traitIdx][3];
        else if (traitValue >= 25)
            return d->traits[traitIdx][2];
        else if (traitValue >= 10)
            return d->traits[traitIdx][1];
        else
            return d->traits[traitIdx][0];
    }
    throw Error::MissingMemoryDefinition("trait", traitIdx);
}

string memory_info::getTraitName(const uint32_t traitIdx) const
{
    if(d->traits.size() > traitIdx)
    {
        return d->traits[traitIdx][d->traits[traitIdx].size()-1];
    }
    throw Error::MissingMemoryDefinition("traitname", traitIdx);
}

std::vector< std::vector<std::string> > const& memory_info::getAllTraits()
{
	return d->traits;
}

string memory_info::getLabor (const uint32_t laborIdx)
{
    if(d->labors.count(laborIdx))
    {
        return d->labors[laborIdx];
    }
    throw Error::MissingMemoryDefinition("labor", laborIdx);
}

std::string memory_info::getMood(const uint32_t moodID)
{
	if(d->moods.size() > moodID)
	{
		return d->moods[moodID];
	}
	throw Error::MissingMemoryDefinition("Mood", moodID);
}

std::string memory_info::PrintOffsets()
{
    ostringstream ss;
    ss << "version:  " << getVersion();
    switch (getOS())
    {
        case OS_LINUX:
            ss << " LINUX" << endl;
            ss << "md5 hash:     " << getString("md5") << endl;
            break;
        case OS_WINDOWS:
            ss << " WINDOWS" << endl;
            ss << "PE timestamp: " << hex << "0x" << getHexValue("pe_timestamp") << endl;
            ss << "md5 hash:     " << getString("md5") << endl;
            break;
        default:
            ss << " UNKNOWN" << endl;
    }
    
    map<string,uint32_t>::const_iterator iter;
    for(iter = d->addresses.begin(); iter != d->addresses.end(); iter++)
    {
        ss << "address " << (*iter).first << " : " << hex << "0x" << (*iter).second << endl;
    }
    map<string,int32_t>::const_iterator iter2;
    for(iter2 = d->offsets.begin(); iter2 != d->offsets.end(); iter2++)
    {
        ss << "offset  " << (*iter2).first << " : " << hex << "0x" << (*iter2).second << endl;
    }
    return ss.str();
}