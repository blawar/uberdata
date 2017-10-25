<?php

$f = fopen('xilinx/xc7z020clg484pkg.csv', 'r');

$i=0;
while($headers = fgetcsv($f))
{
if($i++ < 2) continue;
break;
}

foreach($headers as $i => &$name)
{
	$name = strtolower($name);
	$name = preg_replace('/[^a-zA-Z0-9]+/', '_', $name);
}

$max = (484 / 4);

$i=0;
while($cols = fgetcsv($f))
{
	$row = new stdClass();
	foreach($headers as $j => $name) {
		$row->$name = $cols[$j];
	}

	if(!$row->i_o_type) continue;

	switch(floor($i / $max))
	{
		case 0:
		$y = -$i - 1;
		$x = 0;
		$rotate = 180;
		break;
		case 1:
		$y = -$max - 2;
		$x = ($i % $max) + 1;
		$rotate = 270;
		break;
		case 2:
		$x = $max + 2;
		$y = -($max-($i % $max));
		$rotate = 0;
		break;
		case 3:
		$y = 0;
		$x = ($max-($i % $max));
		$rotate = 90;
		break;
	}
	$i++;

	$x *= 2.54;
	$y *= 2.54;
	

/*
    [pin] => T12
    [pin_name] => DONE_0
    [memory_byte_group] => NA
    [bank] => 0
    [vccaux_group] => NA
    [super_logic_region] => NA
    [i_o_type] => NA
*/

	$buff .= "<pin name=\"$row->pin" . '_' . "$row->pin_name\" x=\"$x\" y=\"$y\" length=\"middle\" rot=\"R$rotate\"/>\n";
}

fclose($f);

$newBuffer = file_get_contents('lib.xml');
$newBuffer = str_replace('%pins%', $buff, $newBuffer);

file_put_contents('zynq7000.xml', $newBuffer);
?>
