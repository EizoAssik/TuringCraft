import Data.Bits
import Data.Word
import Data.ByteString ( pack, writeFile )
import Data.Char ( ord, isAlpha )
import Data.List ( elemIndex )
import Data.List.Split ( wordsBy )
import System.Environment


forceToWord8 x = fromIntegral x :: Word8

startWith :: String -> String -> Bool
startWith [] [] = True
startWith _  [] = True
startWith []  _ = False
startWith (t:ts) (s:ss) = if t == s
                              then ts `startWith` ss
                              else False
-- <=> -> 421
readCMP :: String -> Word8
readCMP tk = forceToWord8 $ foldl1 (+) $ map (\x -> case x of {Nothing -> 0; Just x -> x;}) $ map (\c -> elemIndex c " >= <") tk

-- USIF -> "UISF"
readAthArg :: String -> Word8
readAthArg tk = forceToWord8 $ foldl1 (+) $ map (\x -> case x of {Nothing -> 0; Just x -> x;}) $ map (\c -> elemIndex c "UIS F") tk

pickOpArg token = last $ wordsBy (=='@') token

divideInt :: Int -> Int -> [Word8]
divideInt value bits =
    let lambdas = map (\b -> (\x -> forceToWord8 $ x `shiftR` b))
                      [0, 8..(bits-8)]
    in  map (\f -> f value) lambdas

readImm :: String -> [Word8]
readImm token = 
    let args = map (\x -> read x :: Int) $ wordsBy (=='/') token
    in  divideInt (head args) (last args)

readReg :: Char -> Word8
readReg reg = case elemIndex reg "ABCDEFGH" of
                  Just x  -> forceToWord8 x
                  Nothing -> 0

readArg :: String -> [Word8]
readArg arg
    | elem '/'  arg = readImm arg
    | elem '\'' arg = [forceToWord8.ord $ (read arg :: Char)]
    | otherwise     = [readReg $ head arg]

readDoubleArg :: [String]-> [Word8]
readDoubleArg (arg1:arg2:_) = readArg arg1 ++ readArg arg2

calcImmSetBits :: Int -> Word8
calcImmSetBits len = case len of
                         1 -> 4
                         2 -> 5
                         4 -> 6
                         8 -> 7

dumpOP :: [String] -> [Word8] -> [Word8]
dumpOP [] hexes = reverse hexes 

dumpOP (x:xs) hexes
    | x `startWith` "NOP"    = dumpOP xs (0x00:hexes)
    | x `startWith` "HALT"   = dumpOP xs (0x01:hexes)
    
dumpOP (x:xs) hexes
    | x `startWith` "NOT"    = dumpOP xs ((0x28 .|. readReg arg):hexes)
    | x `startWith` "REV"    = dumpOP xs ((0x30 .|. readReg arg):hexes)
    where arg  = last x

dumpOP (x:xs) hexes
    | x `startWith` "MOD"    = dumpOP rs (argbyte:0x60:hexes)
    | x `startWith` "BOR"    = dumpOP rs (argbyte:0x78:hexes)
    | x `startWith` "BAND"   = dumpOP rs (argbyte:0x70:hexes)
    | x `startWith` "CMP"    = dumpOP rs (argbyte:0xA0:hexes)
    where argtk   = take 2 xs
          rs      = tail $ tail xs
          argv    = readDoubleArg argtk
          argbyte = (head argv) * 8 + (last argv)

dumpOP (x:xs) hexes 
    | x `startWith` "REGCP"  = dumpOP rs (argbyte:(0xC0 .|. reg):hexes)
    where rs      = tail xs
          reg     = readReg $ last x
          argbyte = readReg $ head $ head xs

-- will be REGSET@A 0xFF/8
dumpOP (x:xs) hexes 
    | x `startWith` "REGSET" = dumpOP rs (imm ++ (lenimm:(0xC8 .|. reg):hexes))
    where rs   = tail xs
          reg  = readReg $ last x
          imm  = readArg $ head xs
          lenimm = forceToWord8 $ length imm

-- will be JMP< A | JMP >= B
dumpOP (x:xs) hexes 
    | x `startWith` "JMP"    = dumpOP rs (reg:(0x08 .|. set):hexes)
    where rs   = tail xs
          setr = readCMP x
          set  = if setr == 0 then 7 else setr
          reg  = readReg $ head $ head xs
          
dumpOP (x:xs) hexes
    | x `startWith` "ADD"    = dumpOP rs (argbyte:(0x40 .|. set):hexes)
    | x `startWith` "SUB"    = dumpOP rs (argbyte:(0x48 .|. set):hexes)
    | x `startWith` "MUL"    = dumpOP rs (argbyte:(0x50 .|. set):hexes)
    | x `startWith` "DIV"    = dumpOP rs (argbyte:(0x58 .|. set):hexes)
    | x `startWith` "SFT"    = dumpOP rs (argbyte:(0x68 .|. set):hexes)
    where setr    = readAthArg $ pickOpArg x
          set     = if setr == 0 then 1 else setr
          argtk   = take 2 xs
          rs      = tail $ tail xs
          argv    = readDoubleArg argtk
          argbyte = (head argv) * 8 + (last argv)

dumpOP (x:xs) hexes
    | x `startWith` "DEVR"   = dumpOP xs ((0x90 .|. reg):hexes)
    | x `startWith` "DEVW"   = dumpOP xs ((0x98 .|. reg):hexes)
    where reg = readReg $ last x

-- MEMR@A B; MEMR@B 0xFF/8
dumpOP (x:xs) hexes
    | x `startWith` "MEMR"   = dumpOP rs (argbyte:(0x80 .|. set):hexes) 
    | x `startWith` "MEMW"   = dumpOP rs (argbyte:(0x88 .|. set):hexes)
    where reg     = readReg $ last x
          set     = readReg $ last x
          arg     = head xs
          rs      = tail xs
          argbyte = 0

asm :: String -> [Word8]
asm src = words src `dumpOP` []

main = do
    args     <- getArgs
    let filename = head args
    source   <- readFile filename
    let hex      = asm source
    Data.ByteString.writeFile ("bootstrap.img") (pack hex)
