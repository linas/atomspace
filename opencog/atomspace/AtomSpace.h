/*
 * opencog/atomspace/AtomSpace.h
 *
 * Copyright (C) 2008-2011 OpenCog Foundation
 * Copyright (C) 2002-2007 Novamente LLC
 * Copyright (C) 2015 Linas Vepstas
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _OPENCOG_ATOMSPACE_H
#define _OPENCOG_ATOMSPACE_H

#include <atomic>
#include <iostream>
#include <list>
#include <set>
#include <vector>

#include <opencog/util/async_method_caller.h>
#include <opencog/util/exceptions.h>
#include <opencog/util/oc_omp.h>
#include <opencog/util/RandGen.h>
#include <opencog/util/sigslot.h>

#include <opencog/atoms/atom_types/NameServer.h>
#include <opencog/atoms/truthvalue/TruthValue.h>

#include <opencog/atomspace/TypeIndex.h>

class AtomSpaceUTest;
class AtomTableUTest;
class BasicSaveUTest;

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */
typedef SigSlot<const Handle&> AtomSignal;
typedef SigSlot<const Handle&,
                const TruthValuePtr&,
                const TruthValuePtr&> TVCHSigl;

/**
 * This class provides mechanisms to store atoms and keep indices for
 * efficient lookups. It implements the local storage data structure of
 * OpenCog. It contains methods to add and remove atoms, as well as to
 * retrieve specific sets according to different criteria.
 */
class AtomSpace 
{
#if 0
    friend class Atom;               // Needs to call get_atomtable()
    friend class BackingStore;       // Needs to call get_atomtable()
    friend class StorageNode;        // Needs to call get_atomtable()
    friend class IPFSAtomStorage;    // Needs to call get_atomtable()
    friend class SQLAtomStorage;     // Needs to call get_atomtable()
    friend class UuidSCM;            // Needs to call get_atomtable()
    friend class ::AtomSpaceUTest;
    friend class ::BasicSaveUTest;   // Needs to call get_atomtable()
#endif
    friend class ::AtomTableUTest;   // Needs to call getRandom

    // Debug tools
    static const bool EMIT_DIAGNOSTICS = true;
    static const bool DONT_EMIT_DIAGNOSTICS = false;
    static const bool CHECK_VALUES = true;
    static const bool DONT_CHECK_VALUES = false;

    /**
     * Drop copy constructor and equals operator to
     * prevent accidental copying of large objects.
     */
    AtomSpace& operator=(const AtomSpace&) = delete;
    AtomSpace(const AtomSpace&) = delete;

    // --------------------------------------------------
    // Single, global mutex for locking the indexes.
    // Its recursive because we need to lock twice during atom insertion
    // and removal: we need to keep the indexes stable while we search
    // them during add/remove.
    mutable std::shared_mutex _mtx;

    //! Index of atoms.
    TypeIndex typeIndex;

    bool _transient;
    UUID _uuid;

    /** Find out about atom type additions in the NameServer. */
    NameServer& _nameserver;
    int addedTypeConnection;
    void typeAdded(Type);

    /** Provided signals */
    AtomSignal _addAtomSignal;
    AtomSignal _removeAtomSignal;

    /** Signal emitted when the TV changes. */
    TVCHSigl _TVChangedSignal;

    void clear_all_atoms();
    /// Parent environment for this table.  Null if top-level.
    /// This allows atomspaces to be nested; atoms in this atomspace
    /// can reference those in the parent environment.
    /// The UUID is used to uniquely identify it, for distributed
    /// operation. Viz, other computers on the network may have a copy
    /// of this atomtable, and so need to have its UUID to sync up.
    AtomSpace* _environ;
    std::atomic_int _num_nested;

    bool _read_only;
    bool _copy_on_write;

    Handle getHandle(Type, const std::string&&) const;
    Handle getHandle(Type, const HandleSeq&&) const;
    Handle lookupHandle(const Handle&) const;

    /**
     * Adds an atom to the table.
     *
     * The `force` flag forces the addition of this atom into the
     * atomtable, even if it is already in a parent atomspace.
     *
     * @param The new atom to be added.
     * @return The handle of the newly added atom.
     */
    Handle add(const Handle&, bool force=false, bool do_lock=true);

    /**
     * Return true if the atom table holds this handle, else return false.
     */
    bool holds(const Handle& h) const {
        return (nullptr != h) and h->getAtomSpace() == this;
    }

    /**
     * Return a random atom in the AtomTable.
     * Used in unit testing only.
     */
    Handle getRandom(RandGen* rng) const;

public:
    /**
     * Constructor and destructor for this class.
     *
     * If 'transient' is true, then the resulting AtomSpace is operates
     * in a copy-on-write mode, suitable for holding temporary, scratch
     * results (e.g. for evaluation or inference.) Transient AtomSpaces
     * should have a parent which holds the actual Atoms being worked
     * with.
     */
    AtomSpace(AtomSpace* parent=nullptr, bool transient=false);
    ~AtomSpace();

    UUID get_uuid(void) const { return _uuid; }

    /// Transient atomspaces are lighter-weight, faster, but are missing
    /// some features. They are used during pattern matching, to hold
    /// temporary results. The are always copy-on-write spaces.
    void ready_transient(AtomSpace* parent);
    void clear_transient();

    /// Read-only (RO) atomspaces provide protection against update of the
    /// AtomSpace contents. Atoms in a read-only atomspace canot be
    /// deleted, nor can thier values (including truthvalues) be changed.
    /// New atoms cannot be added to a read-only atomspace.
    void set_read_only(void);
    void set_read_write(void);
    bool get_read_only(void) { return _read_only; }

    /// Copy-on-write (COW) atomspaces provide protection against the
    /// update of the parent atomspace. When an atomspace is marked COW,
    /// it behaves as if it is read-write, but the parent is read-only.
    /// This is convenient for creating temporary atomspaces, wherein
    /// updates will not trash the parent. Transient atomspaces are
    /// always COW.
    void set_copy_on_write(void) { _copy_on_write = true; }
    void clear_copy_on_write(void) { _copy_on_write = false; }
    bool get_copy_on_write(void) { return _copy_on_write; }

    // -------------------------------------------------------

    /// Get the environment that this atomspace was created in.
    AtomSpace* get_environ(void) const { return _environ; }

    /**
     * Return the depth of the Atom, relative to this AtomSpace.
     * The depth is zero, if the Atom is in this table; it is one
     * if it is in the parent, and so on. It is -1 if it is not
     * in the chain.
     */
    int depth(const Handle& atom) const
    {
        if (nullptr == atom) return -1;
        AtomSpace* atab = atom->getAtomSpace();
        const AtomSpace* env = this;
        int count = 0;
        while (env) {
            if (atab == env) return count;
            env = env->_environ;
            count ++;
        }
        return -1;
    }

    /**
     * Return true if the atom is in this atomtable, or if it is
     * in the environment of this atomtable.
     *
     * This is provided in the header file, so that it gets inlined
     * into Atom.cc, where the incoming link is fetched.  This helps
     * avoid what would otherwise be a circular dependency between
     * shared libraries. Yes, this is kind-of hacky, but its the
     * simplest fix for just right now.
     */
    bool in_environ(const Handle& atom) const
    {
        if (nullptr == atom) return false;
        AtomSpace* atab = atom->getAtomSpace();
        const AtomSpace* env = this;
        while (env) {
            if (atab == env) return true;
            env = env->_environ;
        }
        return false;
    }

    /**
     * Compare atomspaces for equality. Useful during testing.
     */
    static bool compare_atomspaces(const AtomSpace& first,
                                   const AtomSpace& second,
                                   bool check_values=CHECK_VALUES,
                                   bool emit_diagnostics=DONT_EMIT_DIAGNOSTICS);
    bool operator==(const AtomSpace& other) const;
    bool operator!=(const AtomSpace& other) const;

    /**
     * Return the number of atoms contained in the space.
     */
    size_t get_size() const;
    size_t get_num_nodes() const;
    size_t get_num_links() const;
    size_t get_num_atoms_of_type(Type type, bool subclass=false) const;

    //! Clear the atomspace, extract all atoms.
    void clear();

    /**
     * Read-write synchronization barrier fence.  When called, this
     * will not return until all the atoms previously added to the
     * atomspace have been fully inserted.
     */
    void barrier(void);

    /**
     * Add an atom to the Atom Table.  If the atom already exists
     * then that is returned.
     */
    Handle add_atom(const Handle&);
    Handle add_atom(const AtomPtr& a)
        { return add_atom(a->get_handle()); }

    /**
     * Add a node to the Atom Table.  If the atom already exists
     * then that is returned.
     *
     * \param t     Type of the node
     * \param name  Name of the node
     */
    Handle add_node(Type, std::string&&);
    Handle xadd_node(Type t, std::string str) {
        return add_node(t, std::move(str));
    }

    /**
     * Add a link to the AtomSpace. If the atom already exists, then
     * that is returned.
     *
     * @param t         Type of the link
     * @param outgoing  a const reference to a HandleSeq containing
     *                  the outgoing set of the link
     */
    Handle add_link(Type, HandleSeq&&);
    Handle xadd_link(Type t, HandleSeq seq) {
        return add_link(t, std::move(seq));
    }

    inline Handle add_link(Type t)
    {
        return add_link(t, HandleSeq{});
    }

    inline Handle add_link(Type t, Handle h)
    {
	    return add_link(t, HandleSeq({h}));
    }

    inline Handle add_link(Type t, Handle ha, Handle hb)
    {
	    return add_link(t, {ha, hb});
    }

    inline Handle add_link(Type t, Handle ha, Handle hb, Handle hc)
    {
        return add_link(t, {ha, hb, hc});
    }

    inline Handle add_link(Type t, Handle ha, Handle hb, Handle hc, Handle hd)
    {
        return add_link(t, {ha, hb, hc, hd});
    }

    inline Handle add_link(Type t, Handle ha, Handle hb, Handle hc,
                           Handle hd, Handle he)
    {
	    return add_link(t, {ha, hb, hc, hd, he});
    }

    inline Handle add_link(Type t, Handle ha, Handle hb, Handle hc,
                           Handle hd, Handle he, Handle hf)
    {
	    return add_link(t, {ha, hb, hc, hd, he, hf});
    }

    inline Handle add_link(Type t, Handle ha, Handle hb, Handle hc,
                           Handle hd, Handle he, Handle hf, Handle hg)
    {
	    return add_link(t, {ha, hb, hc, hd, he, hf, hg});
    }

    inline Handle add_link(Type t, Handle ha, Handle hb, Handle hc,
                           Handle hd, Handle he, Handle hf, Handle hg,
                           Handle hh)
    {
	    return add_link(t, {ha, hb, hc, hd, he, hf, hg, hh});
    }

    inline Handle add_link(Type t, Handle ha, Handle hb, Handle hc,
                           Handle hd, Handle he, Handle hf, Handle hg,
                           Handle hh, Handle hi)
    {
	    return add_link(t, {ha, hb, hc, hd, he, hf, hg, hh, hi});
    }

    /**
     * Given a Value, find all of the Atoms inside of it, and add them
     * to the AtomSpace. Return an equivalent Value, with all Atoms
     * substituted by the ones that are in this AtomSpace.
     */
    ValuePtr add_atoms(const ValuePtr&);

    /**
     * Get an atom from the AtomSpace. If the atom is not there, then
     * return Handle::UNDEFINED.
     */
    Handle get_atom(const Handle&) const;

    /**
     * Extract an atom from the atomspace.  This only removes the atom
     * from the (local, in-RAM) AtomSpace (in this process); any copies
     * of the atom in persistent storage or in other address spaces are
     * unaffected.
     *
     * The atom itself remains valid as long as there are Handles
     * that reference it; the RAM associated with the atom is
     * freed only when the last reference goes away.
     *
     * Note that if the recursive flag is set to false, and the atom
     * appears in the incoming set of some other atom, then extraction
     * will fail.  Thus, it is generally recommended that extraction
     * be recursive, unless you can guarentee that the atom is not in
     * someone else's outgoing set.
     *
     * @param h The Handle of the atom to be removed.
     * @param recursive Recursive-removal flag. If the flag is set,
     *       then this atom, and *everything* that points to it will
     *       be removed from the atomspace.  This can cause a large
     *       cascade of removals!  If the flag is not set, then the
     *       atom will be removed only if its incoming set is empty.
     *       By default, recursion is disabled.
     * @return True if the Atom for the given Handle was successfully
     *         removed. False, otherwise.
     */
    bool extract_atom(const Handle&, bool recursive=true, bool do_lock=true);

    bool remove_atom(const Handle& h, bool recursive=false) {
        return extract_atom(h, recursive);
    }

    /**
     * Set the Value on the atom, performing necessary permissions
     * checking. If this atomspace is read-only, then the setting
     * of values is prohibited.  If this atomspace has read-write
     * permissions, but the atom is in a parent atomspace that is
     * read-only, then the atom is copied into this atomspace, before
     * the value is changed. (Copy-on-write (COW) semantics).
     *
     * If the atom is copied, then the copy is returned.
     */
    Handle set_value(const Handle&, const Handle& key, const ValuePtr& value);
    Handle set_truthvalue(const Handle&, const TruthValuePtr&);

    /**
     * Get a node from the AtomSpace, if it's in there. If the atom
     * can't be found, Handle::UNDEFINED will be returned.
     *
     * @param t     Type of the node
     * @param str   Name of the node
    */
    Handle get_node(Type, std::string&&) const;
    inline Handle get_handle(Type t, std::string str) const {
        return get_node(t, std::move(str));
    }

    /**
     * Get a link from the AtomSpace, if it's in there. If the atom
     * can't be found, Handle::UNDEFINED will be returned.
     *
     * See also the get_atom() method.
     *
     * @param t        Type of the node
     * @param outgoing a reference to a HandleSeq containing
     *        the outgoing set of the link.
    */
    Handle get_link(Type, HandleSeq&&) const;
    inline Handle get_link(Type t, const Handle& ha) const {
        return get_link(t, HandleSeq({ha}));
    }
    Handle get_link(Type t, const Handle& ha, const Handle& hb) const {
        return get_link(t, {ha, hb});
    }
    Handle get_link(Type t, const Handle& ha, const Handle& hb,
                    const Handle& hc) const
    {
        return get_link(t, {ha, hb, hc});
    }
    Handle get_link(Type t, const Handle& ha, const Handle& hb,
                    const Handle& hc, const Handle& hd) const
    {
        return get_link(t, {ha, hb, hc, hd});
    }
    Handle get_handle(Type t, HandleSeq outgoing) const {
        return get_link(t, std::move(outgoing));
    }
    Handle get_handle(Type t, const Handle& ha) const {
	    return get_handle(t, HandleSeq({ha}));
    }
    Handle get_handle(Type t, const Handle& ha, const Handle& hb) const {
	    return get_handle(t, HandleSeq({ha, hb}));
    }

    /**
     * Return true if the handle points to an atom that is in some
     * (any) atomspace; else return false.
     */
    bool is_valid_handle(const Handle& h) const {
        return (nullptr != h) and (h->getAtomSpace() != nullptr);
    }

    /**
     * Gets a set of handles that matches with the given type
     * (subclasses optionally).
     *
     * @param hset the HandleSet into which to insert handles.
     * @param type The desired type.
     * @param subclass Whether type subclasses should be considered.
     *
     * Example of call to this method, which would return all ConceptNodes
     * in the AtomSpace:
     * @code
     *         HandleSet atoms;
     *         atomSpace.get_handlset_by_type(atoms, CONCEPT_NODE);
     * @endcode
     */
    void
    get_handles_by_type(HandleSeq&,
                        Type type,
                        bool subclass=false,
                        bool parent=true,
                        const AtomSpace* = nullptr) const;

    /**
     * Gets a set of handles that matches with the given type,
     * but ONLY if they have an empty incoming set! This might
     * spend more time under the atomtable lock, but should use
     * less RAM when getting large sets, and thus might be faster.
     *
     * @param hset the HandleSet into which to insert handles.
     * @param The desired type.
     * @param Whether type subclasses should be considered.
     *
     * Example of call to this method, which would return all ConceptNodes
     * in the AtomSpace:
     * @code
     *         HandleSet atoms;
     *         atomSpace.get_rootset_by_type(atoms, CONCEPT_NODE);
     * @endcode
     */
    void
    get_root_set_by_type(HandleSeq&,
                         Type type,
                         bool subclass=false,
                         bool parent=true,
                         const AtomSpace* = nullptr) const;

    /**
     * Gets a container of handles that matches with the given type
     * (subclasses optionally).
     * Caution: this is slower than using get_handles_by_type() to
     * get a set, as it forces the use of a copy to deduplicate atoms.
     *
     * @param result An output iterator.
     * @param type The desired type.
     * @param subclass Whether type subclasses should be considered.
     *
     * @return The set of atoms of a given type (subclasses optionally).
     *
     * @note The matched entries are appended to a container whose
     *        OutputIterator is passed as the first argument.
     *
     * Example of call to this method, which would return all entries
     * in AtomSpace:
     * @code
     *         std::list<Handle> ret;
     *         atomSpace.get_handles_by_type(back_inserter(ret), ATOM, true);
     * @endcode
     */
    template <typename OutputIterator> OutputIterator
    get_handleset_by_type(OutputIterator result,
                          Type type,
                          bool subclass=false,
                          bool parent=true) const
    {
        // Sigh. Copy the handles. This hurts performance.
        HandleSeq hset;
        get_handles_by_type(hset, type, subclass, parent);
        return std::copy(hset.begin(), hset.end(), result);
    }

    /** Calls function 'func' on all atoms */
    template <typename Function> void
    foreachHandleByType(Function func,
                        Type type,
                        bool subclass=false,
                        bool parent=true) const
    {
        HandleSeq hset;
        get_handles_by_type(hset, type, subclass, parent);
        std::for_each(hset.begin(), hset.end(),
             [&](const Handle& h)->void {
                  (func)(h);
             });
    }

    template <typename Function> void
    foreachParallelByType(Function func,
                        Type type,
                        bool subclass=false,
                        bool parent=true) const
    {
        HandleSeq hset;
        get_handles_by_type(hset, type, subclass, parent);

        // Parallelize, always, no matter what!
        opencog::setting_omp(opencog::num_threads(), 1);

        OMP_ALGO::for_each(hset.begin(), hset.end(),
             [&](const Handle& h)->void {
                  (func)(h);
             });

        // Reset to default.
        opencog::setting_omp(opencog::num_threads());
    }

    /**
     * Convert the atomspace into a string
     */
    std::string to_string() const;

    /* ----------------------------------------------------------- */
    // ---- Signals

    AtomSignal& atomAddedSignal() { return _addAtomSignal; }
    AtomSignal& atomRemovedSignal() { return _removeAtomSignal; }

    /** Provide ability for others to find out about TV changes */
    TVCHSigl& TVChangedSignal() { return _TVChangedSignal; }
};

/** @}*/
} // namespace opencog

namespace std {

/**
* Output the whole atomspace, actually all orphans as the others are
* output by of their parents (incoming sets).
*/
ostream& operator<<(ostream&, const opencog::AtomSpace&);

} //namespace std

#endif // _OPENCOG_ATOMSPACE_H
