/**
 * ChemTypes.cc
 *
 * Example for defining new Atom Types.
 *
 * The "chem_types.definitions" and "chem_types.inheritance" files
 * are autogenerated; they are created in the build directory when
 * they are created. The full path name is needed, so that they can
 * be found in that directory. These are used by the "atom_types.cc"
 * file to create the required c++ shared library entries.
 */

#include "examples/type-system/demo-types/chem_types.definitions"

#define INHERITANCE_FILE "examples/type-system/demo-types/chem_types.inheritance"
#define INITNAME chem_types_init

#include <opencog/atoms/atom_types/atom_types.cc>
