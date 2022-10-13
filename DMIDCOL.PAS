program DMIDCOL;
{$M $FFF0, 0, $A0000}
uses
  DOS,
  BB7UILIB,
  MOUSER,
  JOYLIB,
  STRINGS;

const
  version='$006';
  ColorPrefsFileName='DOSMID.CPF';

  ColorList:Array[1..11] of string[15] =
  ( 'COLOR_TUI',
    'COLOR_NOTES',
    'COLOR_NOTES_HI',
    'COLOR_TEXT',
    'COLOR_TEMPO',
    'COLOR_CHANS',
    'COLOR_CHANS_DIS',
    'COLOR_PROGRESS1',
    'COLOR_PROGRESS2',
    'COLOR_ERRMSG',
    'SAVE'
  );
type
  colorSet=Array[0..1] of WORD;

  tColorPrefsArray=Array[1..10] of colorSet;

  tColorPrefs=Record
    COLOR_TUI:colorSet;
    COLOR_NOTES:colorSet;
    COLOR_NOTES_HI:colorSet;
    COLOR_TEXT:colorSet;
    COLOR_TEMPO:colorSet;
    COLOR_CHANS:colorSet;
    COLOR_CHANS_DIS:colorSet;
    COLOR_PROGRESS1:colorSet;
    COLOR_PROGRESS2:colorSet;
    COLOR_ERRMSG:colorSet;
  end;

var
   bMouse:boolean;
   colorPrefsArray:tColorPrefsArray;

procedure CheckJoystickAxis(Axis:Byte;
                            Min, Max :Word;
                            CharMin,
                            CharMax:word;
                            var Key:word;
                            var GotResult:Boolean);
const
  ticksMax:byte=9;

var
  Ticks: longint ABSOLUTE $0040:$006c;
  startTicks:longint;
  joyPos:word;

begin

  if not (bJoyEnabled) then exit;

  joyPos := JoystickPosition(Axis);
  if joyPos < Min then
  begin
    StartTicks := ticks;
    while ((JoystickPosition(Axis) < Min) or
          (StartTicks = Ticks)) and
          (Ticks - startTicks < ticksMax) do;
    Key   := CharMin;
    GotResult := true;
  end
  else
  if joyPos > Max then
  begin
    StartTicks := ticks;
    while ((JoystickPosition(Axis) > Max) or
          (StartTicks = Ticks)) and
          (Ticks - startTicks < ticksMax) do;
    Key   := CharMax;
    GotResult := true;
  end
end;

function rightMousePressed(NewKey:word;
                           var Key:word;
                           var GotMouse:Boolean):Boolean;
var
  time:longint;

begin
  rightMousePressed := false;
  if not (bMouse) then exit;
  if (rightpressed) then
  begin
    time := Ticks;
    repeat until not rightpressed;
    if (Ticks - Time < 18) then
    begin
      Key := NewKey;
      GotMouse := true;
      rightMousePressed := true;
    end;
  end;
end;

var
  lastClick:longint;

function leftMousePressed(NewKey:word;
                          NewKeyDC:word;
                          var Key:word;
                          var GotMouse:Boolean):Boolean;
var
  time:longint;
  doubleClick:Boolean;
begin
  leftMousePressed := false;
  if not (bMouse) then exit;
  if (leftpressed) then
  begin
    time := Ticks;
    repeat until not leftpressed;
    if (Ticks - Time > 17) then exit;
    if (Time - Lastclick > 3) then
      Key := NewKey
    else
      Key := NewKeyDC;
    lastClick := ticks;
    GotMouse := true;
    leftMousePressed := true;
  end;
end;

procedure JoyStickMouseCommon(var GotJoystickOrMouse:boolean; var key:word);
{$IFDEF MOUSEWHEEL_READ}
var
  mouseWheel:byte;
{$ENDIF}
begin
  GotJoystickOrMouse := false;


  CheckJoystickButton (JoystickAButton1,
                       Enter,
                       Key,
                       GotJoystickOrMouse);    {Button A}

  CheckJoystickButton (JoystickAButton2,
                       _Esc,
                       Key,
                       GotJoystickOrMouse);    {Button B}

  CheckJoystickButton (JoystickBButton1,
                       _F1,
                       Key,
                       GotJoystickOrMouse);    {Button A controller B}

  CheckJoystickButton (JoystickBButton2,
                       _F7,
                       Key,
                       GotJoystickOrMouse);    {Button A controller B}

  CheckJoystickAxis   (JoystickAAxisY,
                       JoyAxisBCentMin,
                       JoyAxisBCentMax, {Axis   Y}
                       Gray_Up, Gray_Down,
                       Key, GotJoystickOrMouse);

  CheckJoystickAxis   (JoystickAAxisX,
                       JoyAxisBCentMin,
                       JoyAxisBCentMax, {Axis   X}
                       Gray_Left, Gray_Right,
                       Key, GotJoystickOrMouse);

  rightMousePressed(_Esc, Key, GotJoystickOrMouse);
end;

procedure SimpleDialog(dialogStr: string; y:byte; attr:byte);
var
  x:byte;
  width:byte;
begin
  width := ord(dialogStr[0]) + 4;
  x := (80 - width) div 2;
  DrawShadowWin( x, y, width, 3, attr, attr, ' ');
  WriteStr(x + 2, succ(y), dialogStr, attr, 0);
end;

function DoColorPicker(title:string;default:byte):byte;
const
  maxIndex= 16 * 16-1;
  colors_alert = $4f;

var
  x, y, xoffset, yoffset : byte;
  index,oldindex:byte;
  str:string[4];
  key:word;
  blink:Boolean;
  GotJoystickOrMouse:Boolean;
  t:longint;
  bite,color:byte;

begin

  str[0]:=char(4);
  str[1]:=' ';
  str[2]:=' ';
  str[3]:=' ';
  str[4]:='h';

  x:=0;
  y:=0;
  blink:=false;
  setBlinkMode(word(blink));
  xoffset := 13;
  yoffset := 7;

  DrawShadowWin(pred(xoffset), yoffset -1, 66, 18, Colors_Alert, Colors_Alert, ' ');
  WriteStr(xoffset + 30 - (ord(title[0]) div 2), pred(yOffset), '[ ' + title + ' ]' , Colors_Alert, 0);
  WriteStr((xoffset) + 66 div 2 - 4, yoffset + 16, '[    ]', Colors_Alert, 0);

  for x := 0 to $f do
    for y := 0 to $f do
    begin
      str[2]:=hexStringLow[succ(x)];
      str[3]:=hexStringLow[succ(y)];
      WriteStr((x * 4) + xoffset,y + yoffset, str, (x shl 4) or y, 0);
    end;

  index    := ((default mod 16) shl 4) + (default div 16);
  oldindex := index +1;
  str[0] := #4;
  str[3] := 'h';
  str[4] := ' ';

  repeat
    begin
      repeat
        if(index <> oldindex) then
        begin
          x := (oldindex mod 16) * 4 + xoffset;
          y := (oldindex div 16) + yoffset;
          Mem[videoSeg:160 * y + x + x] := ord(' ');
          x := (index mod 16) * 4 + xoffset;
          y := (index div 16) + yoffset;
          Mem[videoSeg:160 * y + x + x] := $10; {ord('*');}
          oldindex := index;
          color := index mod 16;
          color := color shl 4;
          color := color + (index div 16);
          str[1] := hexStringLow[((color and $f0) shr 4)+1];
          str[2] := hexStringLow[(color and $0f) + 1];
          WriteStr(xoffset + 30
          ,yoffset + 16, str, color,4);
        end;

       GotJoystickOrMouse := false;
       JoyStickMouseCommon(GotJoystickOrMouse, key);

       if(leftMousePressed(no_key, Enter, Key, GotJoystickOrMouse)) then
       begin
          y := getmouseY - yOffset;
          x := getmouseX - Xoffset;
          if ( x >= 0 ) and (x < 64) and
             ( y >= 0 ) and (y < 16) then
          begin
             index := (16 * y) + (x div 4);
          end;
       end;

      until keypressed or GotJoystickOrMouse;
      if not (GotJoystickOrMouse) then
         key:=ext_readkey;
      case (key) of
        _f3 :begin
                   {blink:=not blink;         }
                   {setBlinkMode(word(blink));}
                 end;
        _right,
        gray_right: if(index < maxIndex) then
                      inc(index,1);
        _left,
        gray_left:  if(index > 0) then
                      dec(index,1);
        _up,
        gray_up:    if(index >= 16) then
                      dec(index, 16);
        _down,
        gray_down:  if(index + 16 < maxIndex) then
                      inc(index, 16);
      end;

   end;
   until (key = _esc) or (key = Enter);
   if (key = _esc) then
     DOcolorPicker := default
   else
     DoColorPicker := color;

end;

procedure DumpKBandMouse;
begin
  repeat
    if (keypressed) then
         ext_readkey;
  until not keypressed and (not(bMouse) or not(rightpressed));
end;

procedure DoDelay;
var
  time:longint;

begin
  time := ticks;
  repeat until (ticks - time > 5);
end;

procedure DOPause;
var
  GotJoystickOrMouse:Boolean;
  key:word;
begin
  repeat
    GotJoystickOrMouse := false;
    JoyStickMouseCommon(GotJoystickOrMouse, key);
    leftMousePressed(Enter, Enter, Key, GotJoystickOrMouse);
  until keypressed or GotJoystickOrMouse;
  DumpKBandMouse;
  SetBlinkMode(0);
end;

function LoadColorPrefs:boolean;
var
  colorPrefsFile:File of tColorPrefsArray;

begin
  Assign(colorPrefsFile, colorPrefsFileName);
  {$I-}
  reset(colorPrefsFile);
  read(colorPrefsFile, colorPrefsArray);
  close(colorPrefsFile);
  {$I+}
  if IOResult <> 0 then
  begin
    LoadColorPrefs := FALSE;
    GURU_MEDITATION('Not found!', colorPrefsFileName);
    doPause;
  end
  else
    LoadColorPrefs := TRUE;
end;

function SaveColorPrefs:boolean;
var
  colorPrefsFile:File of tColorPrefsArray;

begin
  Assign(colorPrefsFile, colorPrefsFileName);
  {$I-}
  reset(colorPrefsFile);
  write(colorPrefsFile, colorPrefsArray);
  close(colorPrefsFile);
  {$I+}
  if IOResult <> 0 then
  begin
    SaveColorPrefs := FALSE;
    GURU_MEDITATION('File not saved!', colorPrefsFileName);
    doPause;
  end
  else
    SaveColorPrefs := TRUE;
end;

procedure DoMainMenu;
const
  attrTXT=$0f;
  maxIdx=11;
var
  x,y,x2,y2:byte;
  GotJoystickOrMouse:boolean;
  key:word;
  idx, idxOld, bwColor, bwColorOld:byte;

  procedure redrawList;
  var
    y:byte;
  begin
    ClrScr(' ', $07);
    for y := 1 to maxIdx do
    begin
       WriteStr(x, y, ColorList[y], attrTXT, 0);
       if(y < maxIdx) then
       begin
         WriteStr(x+20, y, '##########', colorPrefsArray[y][1] shr 8, 0);
         WriteStr(x+35, y, '##########', colorPrefsArray[y][0] shr 8, 0);
       end;
    end;
    HilightLine(24, $1f);
    WriteStrCen(24, 'DOSMID Color Config BBond007 Ver ' + version, $1f, 0);
  end;

begin
  x := 1;
  y := 1;
  idx := 1;
  idxOld := 0;
  bwColor := 1;
  bwColorOld := 1;
  redrawList;
  repeat
    repeat
      if(idx <> idxOld) or (bwColor <> bwColorOld) then
      begin
        if (bwColor = 0) then
          x2 := x + 31
        else
          x2 := x + 16;
        if (idx < maxIDx) then
          WriteStr(x2, idx, '-->', $4f, 0)
        else
          WriteStr(x2, idx, '<--', $4f, 0);
         if (bwColorOld = 0) then
          x2 := x + 31
        else
          x2 := x + 16;
        if (idxOld <> 0) then
           WriteStr(x2, idxOld, '   ', $07, 0);
        idxOld := idx;
        bwColorOld := bwColor;
      end;
      JoystickMouseCommon(GotJoystickOrMouse, key);
      if(leftMousePressed(no_key, Enter, Key, GotJoystickOrMouse)) then
      begin
         y2 := getmouseY;
         x2 := getmouseX;
         if ( y2 >= 1 ) and (y2 <= maxIdx) then
         begin
            if ( x2 >= 0 ) and (x2 < 31) then
              bwColor := 1
            else
              bwColor := 0;
            idx := y2;
         end;
      end;

    until keypressed or GotJoystickOrMouse;
    if not(GotJoystickOrMouse) then
       if keypressed then
         key := ext_readkey;

    case (key) of

         _right,
        gray_right: bwColor := 0;
        _left,
        gray_left:  bwColor := 1;

        _up,
        gray_up:    if(idx > 1) then
                      dec(idx,1);
        _down,
        gray_down:  if(idx < maxIdx) then
                      inc(idx,1);
        Enter: begin
                 if (idx < 11) then
                 begin
                   colorPrefsArray[idx][bwColor] :=
                   DoColorPicker(ColorList[idx],colorPrefsArray[idx][bwColor] shr 8) shl 8;
                 end
                 else
                 begin
                   SimpleDialog('Saving...', 12,  $4f);
                   SaveColorPrefs;
                   DoDelay;
                 end;
                 redrawList;
                 idxOld := 0;
               end;
    end;
  until key = _esc;
end;

var
  _memavail:longint;
  tmp:string[20];

begin
  {memory leak detection -- this MUST be the first line of the program}
  _memavail:=memavail;
  setBlinkMode(0);

  if(loadColorPrefs) then
  begin
    CsrOff;
    calibrateJoystick;
    bMouse := mouseInstalled;
    if bMouse then
       showmouse;
    DoMainMenu;
    ClrScr(' ', $07);
    if bMouse then
      hideMouse;
    CsrOn;
  end;
  {memory leak detection -- these MUST be the last lines of the program}
  if _memavail<>memavail then
  begin
    Str(_memavail-memavail, tmp);
    GURU_MEDITATION('Memory leak detected  '+ #26 + ' ' + tmp + ' bytes', '');
  end;
  setBlinkMode(1);
end.
