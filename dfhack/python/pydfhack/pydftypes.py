from ctypes import *
from collections import namedtuple

Position2D = namedtuple("Position2D", "x, y")
Position3D = namedtuple("Position3D", "x, y, z")
Rectangle = namedtuple("Rectangle", "x1, y1, x2, y2")
Note = namedtuple("Note", "symbol, foreground, background, name, position")
Construction = namedtuple("Construction", "position, form, unk_8, mat_type, mat_idx, unk3, unk4, unk5, unk6, origin")
Vein = namedtuple("Vein", "vtable, type, flags, address, assignment")
FrozenLiquidVein = namedtuple("FrozenLiquidVein", "vtable, address, tiles")
SpatterVein = namedtuple("SpatterVein", "vtable, address, mat1, unk1, mat2, mat3, intensity")
Settlement = namedtuple("Settlement", "origin, name, world_pos, local_pos")
Attribute = namedtuple("Attribute", "level, field_4, field_8, field_C, leveldiff, field_14, field_18");
Skill = namedtuple("Skill", "id, experience, rating")
Tree = namedtuple("Tree", "type, material, position, address")
CreatureCaste = namedtuple("CreatureCaste", "rawname, singular, plural, adjective")
CreatureTypeEx = namedtuple("CreatureTypeEx", "rawname, castes, tile_character, tilecolor")
TileColor = namedtuple("TileColor", "fore, back, bright")
Name = namedtuple("Name", "first_name, nickname, language, has_name, words, parts_of_speech")

char_array = c_char * 128

class Soul(object):
    def __init__(self, *args, **kwds):
        if kwds:
            for k, v in kwds.iteritems():
                self.__dict__[k] = v

class MapBlock40d(object):
    pass

class ViewScreen(Structure):
    _fields_ = [("type", c_int)]

class Matgloss(Structure):
    _fields_ = [("id", char_array),
                ("fore", c_byte),
                ("back", c_byte),
                ("bright", c_byte),
                ("name", char_array)]

class Descriptor_Color(Structure):
    _fields_ = [("id", char_array),
                ("r", c_float),
                ("v", c_float),
                ("b", c_float),
                ("name", char_array)]

class MatglossOther(Structure):
    _fields_ = [("rawname", char_array)]
