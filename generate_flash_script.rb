#!/usr/bin/env ruby
require 'erb'

File.write('.flash.jlink', ERB.new(DATA.read).result);

__END__
device NRF51822

// Unlock flash
//w4 0x4001e504, 2
// Mass erase
//w4 0x4001e50c, 1

loadbin <%= ARGV[0] %>, 0

r

qc

