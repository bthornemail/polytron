module PolytronModel where

import Data.Bits ((.&.), rotateL, rotateR, xor)
import Data.Word (Word8, Word16, Word32, Word64)

data PolytronProcessorMode = VectorScribeMode | ParallelTileMode
  deriving (Eq, Show, Enum, Bounded)

data PolytronCoreRegister = PolytronCoreRegister
  { instructionOpcode :: !Word8
  , coreAddressLink :: !Word32
  , clockCycleWeight :: !Word16
  } deriving (Eq, Show)

data PolytronContext = PolytronContext
  { activeHardwareAxis :: !Word64
  , currentWordContext :: !Word64
  } deriving (Eq, Show)

executePolytronStep :: PolytronContext -> Word64 -> (PolytronContext, Word64)
executePolytronStep ctx inboundToken =
  let base4320 = inboundToken `rem` 4320
      surface4320_2 = base4320 * 4320
      horizon4320_4 = surface4320_2 * 4320 * 4320
      leftTorque = rotateL inboundToken 1
      rightTorque = rotateR (activeHardwareAxis ctx) 2
      blended = leftTorque `xor` rightTorque `xor` horizon4320_4
      nextCoordinate = blended `xor` 0x1D1D1D1D1D1D1D1D
      nextContext = PolytronContext
        { activeHardwareAxis = nextCoordinate
        , currentWordContext = nextCoordinate .&. 0xFFFF
        }
  in (nextContext, nextCoordinate)

data SubstrateTarget = TargetRules | TargetFacts | TargetClosures | TargetCombinators | TargetCons
  deriving (Eq, Show, Enum, Bounded)
