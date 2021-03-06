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

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include "DFExport.h"
#include <string>
#include <sstream>
#include <exception>

namespace DFHack
{
    namespace Error
    {
        class DFHACK_EXPORT NoProcess : public std::exception
        {
        public:
            virtual const char* what() const throw()
            {
                return "couldn't find a suitable process";
            }
        };

        class DFHACK_EXPORT CantAttach : public std::exception
        {
        public:
            virtual const char* what() const throw()
            {
                return "couldn't attach to process";
            }
        };

        class DFHACK_EXPORT NoMapLoaded : public std::exception
        {
        public:
            virtual const char* what() const throw()
            {
                return "no map has been loaded in the dwarf fortress process";
            }
        };

        class DFHACK_EXPORT BadMapDimensions : public std::exception
        {
        public:
            BadMapDimensions(uint32_t& _x, uint32_t& _y) : x(_x), y(_y) {}
            const uint32_t x;
            const uint32_t y;

            virtual const char* what() const throw()
            {
                return "both x and y needs to be between 0 and 48";
            }
        };

        // a call to DFHack::mem_info::get* failed
        class DFHACK_EXPORT MissingMemoryDefinition : public std::exception
        {
        public:
            MissingMemoryDefinition(const char* _type, const char* _key) : type(_type), key(_key)
            {
                std::stringstream s;
                s << "memory definition missing: type " << type << " key " << key;
                full = s.str();
            }
            // Used by functios using integer keys, such as getTrait
            MissingMemoryDefinition(const char* _type, uint32_t _key) : type(_type)
            {
                std::stringstream s1;
                s1 << _key;
                key = s1.str();
                
                std::stringstream s;
                s << "memory definition missing: type " << type << " key " << key;
                full = s.str();
            }
            virtual ~MissingMemoryDefinition() throw(){};

            // (perhaps it should be an enum, but this is intended for easy printing/logging)
            // type can be any of the following:
            //
            // address
            // offset
            // hexvalue
            // string
            // profession
            // job
            // skill
            // trait
            // traitname
            // labor
            std::string full;
            const std::string type;
            std::string key;

            virtual const char* what() const throw()
            {
                return full.c_str();
            }
        };

        // Syntax errors and whatnot, the xml cant be read
        class DFHACK_EXPORT MemoryXmlParse : public std::exception
        {
        public:
            MemoryXmlParse(const char* _desc, int _id, int _row, int _col)
            :desc(_desc), id(_id), row(_row), col(_col)
            {
                std::stringstream s;
                s << "error " << id << ": " << desc << ", at row " << row << " col " << col;
                full = s.str();
            }
                
                
            std::string full;
            const std::string desc;
            const int id;
            const int row;
            const int col;
            
            virtual ~MemoryXmlParse() throw(){};

            virtual const char* what() const throw()
            {
                return full.c_str();
            }
        };

        class DFHACK_EXPORT MemoryXmlBadAttribute : public std::exception
        {
        public:
            MemoryXmlBadAttribute(const char* _attr) : attr(_attr)
            {
                std::stringstream s;
                s << "attribute is either missing or invalid: " << attr;
                full = s.str();
            }
            std::string full;
            std::string attr;
            
            virtual ~MemoryXmlBadAttribute() throw(){};

            virtual const char* what() const throw()
            {
                return full.c_str();
            }
        };

        class DFHACK_EXPORT MemoryXmlNoRoot : public std::exception
        {
        public:
            MemoryXmlNoRoot() {}
            
            virtual ~MemoryXmlNoRoot() throw(){};

            virtual const char* what() const throw()
            {
                return "no pElem found";
            }
        };

        class DFHACK_EXPORT MemoryXmlNoDFExtractor : public std::exception
        {
        public:
            MemoryXmlNoDFExtractor(const char* _name) : name(_name)
            {
                std::stringstream s;
                s << "DFExtractor != " << name;
                full = s.str();
            }
            virtual ~MemoryXmlNoDFExtractor() throw(){};

            std::string name;
            std::string full;

            virtual const char* what() const throw()
            {
                return full.c_str();
            }
        };

        class DFHACK_EXPORT MemoryXmlUnderspecifiedEntry : public std::exception
        {
        public:
            MemoryXmlUnderspecifiedEntry(const char * _where) : where(_where)
            {
                std::stringstream s;
                s << "underspecified MemInfo entry, each entry needs to set both the name attribute and have a value. parent: " << where;
                full = s.str();
            }
            virtual ~MemoryXmlUnderspecifiedEntry() throw(){};
            std::string where;
            std::string full;
            virtual const char* what() const throw()
            {
                return full.c_str();
            }
        };

        class DFHACK_EXPORT MemoryXmlUnknownType : public std::exception
        {
        public:
            MemoryXmlUnknownType(const char* _type) : type(_type)
            {
                std::stringstream s;
                s << "unknown MemInfo type: " << type;
                full = s.str();
            }
            virtual ~MemoryXmlUnknownType() throw(){};

            std::string type;
            std::string full;

            virtual const char* what() const throw()
            {
                return full.c_str();
            }
        };
        
        class DFHACK_EXPORT SHMServerDisappeared : public std::exception
        {
        public:
            SHMServerDisappeared(){}
            virtual ~SHMServerDisappeared() throw(){};
            virtual const char* what() const throw()
            {
                return "The server process has disappeared";
            }
        };
        class DFHACK_EXPORT SHMLockingError : public std::exception
        {
        public:
            SHMLockingError(const char* _type) : type(_type)
            {
                std::stringstream s;
                s << "SHM locking error: " << type;
                full = s.str();
            }
            virtual ~SHMLockingError() throw(){};
            
            std::string type;
            std::string full;
            
            virtual const char* what() const throw()
            {
                return full.c_str();
            }
        };
        class DFHACK_EXPORT MemoryAccessDenied : public std::exception
        {
        public:
            MemoryAccessDenied() {}
            virtual ~MemoryAccessDenied() throw(){};
            virtual const char* what() const throw()
            {
                return "SHM ACCESS DENIED";
            }
        };
        class DFHACK_EXPORT SHMVersionMismatch : public std::exception
        {
        public:
            SHMVersionMismatch() {}
            virtual ~SHMVersionMismatch() throw(){};
            virtual const char* what() const throw()
            {
                return "SHM VERSION MISMATCH";
            }
        };
        class DFHACK_EXPORT SHMAttachFailure : public std::exception
        {
        public:
            SHMAttachFailure() {}
            virtual ~SHMAttachFailure() throw(){};
            virtual const char* what() const throw()
            {
                return "SHM ATTACH FAILURE";
            }
        };
        class DFHACK_EXPORT ModuleNotInitialized : public std::exception
        {
        public:
            ModuleNotInitialized() {}
            virtual ~ModuleNotInitialized() throw(){};
            virtual const char* what() const throw()
            {
                return "Programmer error: module not initialized!";
            }
        };
        
    }
}

#endif // ERROR_H_INCLUDED
