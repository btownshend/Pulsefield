echo on
oscshow();
oscmsgout('TO','/test',{});
oscmsgout('LD','/test',{});
oscshow();
m=oscmsgin('MPV')
m=oscmsgin('MPO')
m=oscmsgin('MPL')
oscmsgout('MPV','/test',{});
m=oscmsgin('MPV',1.0)
oscshow();
oscclose
oscshow();
echo off
