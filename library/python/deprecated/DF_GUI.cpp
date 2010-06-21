/*
www.sourceforge.net/projects/dfhack
Copyright (c) 2009 Petr Mrázek (peterix), Kenneth Ferland (Impaler[WrG]), dorf, doomchild

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

#ifndef __DFGUI__
#define __DFGUI__
#include "Tranquility.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "Python.h"
#include "integers.h"
#include "DFTypes.h"
#include "modules/Gui.h"

using namespace DFHack;

struct DF_GUI
{
	PyObject_HEAD
	DFHack::Gui* g_Ptr;
};

static PyObject* DF_GUI_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DF_GUI* self;
	
	self = (DF_GUI*)type->tp_alloc(type, 0);
	
	if(self != NULL)
		self->g_Ptr = NULL;
	
	return (PyObject*)self;
}

static int DF_GUI_init(DF_GUI* self, PyObject* args, PyObject* kwds)
{
	return 0;
}

static void DF_GUI_dealloc(DF_GUI* self)
{
	PySys_WriteStdout("gui dealloc\n");
	
	if(self != NULL)
	{
		PySys_WriteStdout("gui not NULL\n");
		
		if(self->g_Ptr != NULL)
		{
			PySys_WriteStdout("g_Ptr = 0x%x\n", self->g_Ptr);
			
			delete self->g_Ptr;
			
			PySys_WriteStdout("g_Ptr deleted\n");
			
			self->g_Ptr = NULL;
		}
		
		self->ob_type->tp_free((PyObject*)self);
	}
	
	PySys_WriteStdout("gui dealloc done\n");
}

// Type methods

static PyObject* DF_GUI_Start(DF_GUI* self, PyObject* args)
{
	if(self->g_Ptr != NULL)
	{
		if(self->g_Ptr->Start())
			Py_RETURN_TRUE;
		else
			Py_RETURN_FALSE;
	}
	
	Py_RETURN_NONE;
}

static PyObject* DF_GUI_Finish(DF_GUI* self, PyObject* args)
{
	if(self->g_Ptr != NULL)
	{
		if(self->g_Ptr->Finish())
			Py_RETURN_TRUE;
		else
			Py_RETURN_FALSE;
	}
	
	Py_RETURN_NONE;
}

static PyObject* DF_GUI_ReadViewScreen(DF_GUI* self, PyObject* args)
{
	DFHack::t_viewscreen viewscreen;
	
	if(self->g_Ptr != NULL)
	{
		if(self->g_Ptr->ReadViewScreen(viewscreen))
			return PyInt_FromLong(viewscreen.type);
	}
	
	Py_RETURN_NONE;
}

static PyMethodDef DF_GUI_methods[] =
{
	{"Start", (PyCFunction)DF_GUI_Start, METH_NOARGS, ""},
	{"Finish", (PyCFunction)DF_GUI_Finish, METH_NOARGS, ""},
	{"Read_View_Screen", (PyCFunction)DF_GUI_ReadViewScreen, METH_NOARGS, ""},
	{NULL}	//Sentinel
};

// Getter/Setter

static PyObject* DF_GUI_getPauseState(DF_GUI* self, void* closure)
{
	if(self->g_Ptr != NULL)
	{
		if(self->g_Ptr->ReadPauseState())
			Py_RETURN_TRUE;
		else
			Py_RETURN_FALSE;
	}
	
	Py_RETURN_NONE;
}

static PyObject* DF_GUI_getMenuState(DF_GUI* self, void* closure)
{
	if(self->g_Ptr != NULL)
	{
		return PyInt_FromLong(self->g_Ptr->ReadMenuState());
	}
	
	Py_RETURN_NONE;
}

static PyGetSetDef DF_GUI_getterSetters[] =
{
    {"paused", (getter)DF_GUI_getPauseState, NULL, "paused", NULL},
	{"menu_state", (getter)DF_GUI_getMenuState, NULL, "menu_state", NULL},
    {NULL}  // Sentinel
};

static PyTypeObject DF_GUI_type =
{
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydfhack._GUIManager",             /*tp_name*/
    sizeof(DF_GUI), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)DF_GUI_dealloc,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "pydfhack GUIManager object",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    DF_GUI_methods,             /* tp_methods */
    0,                      /* tp_members */
    DF_GUI_getterSetters,      /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)DF_GUI_init,      /* tp_init */
    0,                         /* tp_alloc */
    DF_GUI_new,                 /* tp_new */
};

#endif
