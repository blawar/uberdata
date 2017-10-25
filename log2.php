<?php
$buffer = <<<EAD
#ifndef LOG2_H
#define LOG2_H
#define LOG2(n) (
EAD;

for($i=32; $i > 0; $i--)
{
	$buffer .= "(n)&(1<<" . ($i-1) . ")?$i:";
}

$buffer .= <<<EAD
0)

#define MAXVALUE(n)  (
EAD;

for($i=1; $i <= 4; $i++)
{
$buffer .= "$i == sizeof(n) ? 0x" . str_repeat('f',$i*2) . ":";
}

$buffer .= <<<EAD
0)

#endif

EAD;

file_put_contents('log2.h', $buffer);
