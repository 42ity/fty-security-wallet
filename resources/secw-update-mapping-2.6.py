#!/usr/bin/python3
#   =========================================================================
#   Copyright (C) 2014 - 2021 Eaton
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#   =========================================================================
#
#   This script updates a security wallet mapping file (json) to upgrade onto IPM2 2.6.0
#   It takes as arguments:
#       - the input secw mapping file (-i)
#       - the output secw mapping file (-o)
#   Note: the output file can be the same as input.
#   Returns 0 if success, else !=0
#

import sys
import traceback
import getopt
import json

def usage():
    print(sys.argv[0] + " [-h] [-v|-V] -i <input-mapping-file> [-o <output-mapping-file>]")

g_inputFile = ""
g_outputFile = ""
g_verbose = False
g_debug = False

#get args
try:
    opts, args = getopt.getopt(sys.argv[1:],"vVhi:o:")
    for opt, arg in opts:
        if opt == '-h':
            usage()
            sys.exit(0)
        elif opt == "-i":
            g_inputFile = arg
        elif opt == "-o":
            g_outputFile = arg
        elif opt == "-v":
            g_verbose = True
        elif opt == "-V":
            g_verbose = True
            g_debug = True
        else:
            usage()
            sys.exit(1)
except getopt.GetoptError as e:
    print(e)
    usage()
    sys.exit(1)

if g_debug:
    print("Input file: " + g_inputFile)
    print("Output file: " + g_outputFile)

#check args
if g_inputFile == "":
    sys.stderr.write("ERROR: Input file is required\n")
    usage()
    sys.exit(1)

#read input file, set data (json)
data = None
try:
    if g_verbose:
        print("Reading '" + g_inputFile + "'...")

    fileIn = open(g_inputFile, "r")
    data = json.load(fileIn)
    fileIn.close()

    if g_debug:
        print("Input mappings (size: " + str(len(data['mappings'])) + "):")
        json.dump(data['mappings'], sys.stdout, indent=4)
        print()
except Exception as e:
    print(e)
    traceback.print_exc()
    sys.exit(1)

#inspect mass-manager data mappings, set http(s)_assetDict
https_assetDict = dict()
http_assetDict = dict()
try:
    if g_debug:
        print("Mapping inspection...")

    for it in data['mappings']:
        if it["cam_service"] == "mass_device_management":
            asset = it["cam_asset"]
            protocol = it["cam_protocol"]

            if protocol == "https":
                https_assetDict[asset] = it["cam_port"]
            elif protocol == "http":
                http_assetDict[asset] = it["cam_port"]

    if g_debug:
        print("https_assetDict: " + str(https_assetDict))
        print("http_assetDict: " + str(http_assetDict))
except Exception as e:
    print(e)
    traceback.print_exc()
    sys.exit(1)

#check mass-manager data mapping entries
#change protocol 'http'/https' to 'mass_management'
#change single 'http' port to 443 default
#delete coupled 'http' entry (obsolete)
try:
    mappingChanged = False
    indexToRemoveList = []
    portChangedCnt = 0
    index = -1
    for it in data['mappings']:
        index = index + 1
        if it["cam_service"] == "mass_device_management":
            asset = it["cam_asset"]
            protocol = it["cam_protocol"]

            if protocol == "https":
                it["cam_protocol"] = "mass_management"
                mappingChanged = True
            elif protocol == "http":
                if asset not in https_assetDict.keys():
                    #single http
                    it["cam_protocol"] = "mass_management"
                    it["cam_port"] = 443
                    portChangedCnt = portChangedCnt + 1
                    mappingChanged = True
                else:
                    #coupled http
                    it["cam_service"] += "_TO_REMOVE"
                    mappingChanged = True
                    indexToRemoveList.append(index)

    if g_verbose:
        print("Mapping changed: " + str(mappingChanged) \
              + " (port changed: " + str(portChangedCnt) \
              + ", removed: " + str(len(indexToRemoveList)) \
              + ")")

    if g_debug:
        print("indexToRemoveList: " + str(indexToRemoveList))

    #delete obsolete entries from data mapping
    for index in reversed(indexToRemoveList):
        data['mappings'].pop(index)
except Exception as e:
    print(e)
    traceback.print_exc()
    sys.exit(1)

#here, changes on data are done

if g_debug:
    print("Output mappings (size: " + str(len(data['mappings'])) + "):")

if g_debug or g_outputFile == "":
    json.dump(data['mappings'], sys.stdout, indent=4)
    print()

#write data to output file
try:
    if g_outputFile != "":
        if g_verbose:
            print("Writing '" + g_outputFile + "'...")
        fileOut = open(g_outputFile, "w")
        json.dump(data, fileOut, indent=4)
        fileOut.flush()
        fileOut.close()
except Exception as e:
    print(e)
    traceback.print_exc()
    sys.exit(1)

if g_verbose:
    print("Done")

sys.exit(0)
