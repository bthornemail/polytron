module Main where

import Data.Bits ((.&.))
import PolytronModel
import System.Exit (exitFailure, exitSuccess)

checkProcessorModes :: Bool
checkProcessorModes = length [VectorScribeMode .. ParallelTileMode] == 2

checkRegisterBoundaries :: Bool
checkRegisterBoundaries =
  let reg = PolytronCoreRegister 0x7F 0xABCDEFFF 12
  in instructionOpcode reg == 0x7F
     && coreAddressLink reg == 0xABCDEFFF
     && clockCycleWeight reg == 12

checkHardwareTransformInvariants :: Bool
checkHardwareTransformInvariants =
  let initialContext = PolytronContext 0x1337133713371337 0x0000
      token = 0xDEADBEEFDEADBEEF
      (ctx1, coord1) = executePolytronStep initialContext token
      (ctx2, coord2) = executePolytronStep initialContext token
  in coord1 == coord2
     && activeHardwareAxis ctx1 == activeHardwareAxis ctx2
     && currentWordContext ctx1 == (coord1 .&. 0xFFFF)

checkSubstrateTracks :: Bool
checkSubstrateTracks = length [TargetRules .. TargetCons] == 5

main :: IO ()
main = do
  putStrLn "Executing Layer 7.5 Polytron Co-Processor Invariant Scan..."
  let assertions =
        [ (checkProcessorModes, "Processor Execution Modes Representation Completeness")
        , (checkRegisterBoundaries, "Rigid Co-Processor Core Layout State Mappings")
        , (checkHardwareTransformInvariants, "Deterministic Hardware Step and 16-Bit Masking")
        , (checkSubstrateTracks, "Five Core Binary Substrate Targets Verification")
        ]
  mapM_ (\(passed, label) ->
    putStrLn $ (if passed then "  [PASS] " else "  [FAIL] ") ++ label) assertions
  if all fst assertions then exitSuccess else exitFailure
