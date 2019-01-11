#!/bin/sh

squish -l struct.fr
squish -l modules2.fr
squish -l rtxcross.fr
squish -l far2.fr

squish packbits2.fr
