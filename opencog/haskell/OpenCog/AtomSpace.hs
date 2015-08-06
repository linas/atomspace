-- GSoC 2015 - Haskell bindings for OpenCog.
{-# LANGUAGE TypeOperators        #-}

-- | This library defines Haskell Bindings for the AtomSpace.
module OpenCog.AtomSpace
    (
    -- * AtomSpace Environment
      AtomSpace
    , runOnNewAtomSpace
    -- * AtomSpace Interaction
    , insert
    , remove
    , get
    , debug
    -- * AtomSpace Printing
    , printAtom
    , showAtom
    -- * AtomSpace Main Data Types
    , TruthVal (..)
    , noTv
    , withTv
    , AtomName (..)
    , Atom (..)
    , AtomGen (..)
    , appAtomGen
    -- * AtomSpace Phantom Types
    , AtomType(..)
    , type (<~)
    ) where

import OpenCog.AtomSpace.Api
import OpenCog.AtomSpace.Types
import OpenCog.AtomSpace.Env         (AtomSpace,runOnNewAtomSpace)
import OpenCog.AtomSpace.Utils       (printAtom,showAtom)
import OpenCog.AtomSpace.AtomType    (AtomType(..))
import OpenCog.AtomSpace.Inheritance (type (<~))
