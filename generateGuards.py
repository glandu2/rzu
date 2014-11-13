import re
import sys

checkIfndef = re.compile(r'#ifndef [_A-Z0-9]+_H_?$')
checkDefine = re.compile(r'#define [_A-Z0-9]+_H_?$')
checkEndif = re.compile(r'#endif (//|/\*) [_A-Z0-9]+_H_?( \*/)?$')

for filename in sys.stdin:
	filename = re.sub("[\r\n]", "", filename)

	guard = filename.upper()
	guard = re.sub("[^A-Z0-9]", "_", guard)
	guard = re.sub("^_+", "", guard)

	with open(filename) as headerFile:
		fileLines = headerFile.readlines()

	fileLines = [ re.sub("[\r\n]", "", line) for line in fileLines ]

	numberDetect = 0

	if(checkIfndef.match(fileLines[0])):
		fileLines[0] = "#ifndef " + guard
		numberDetect += 1

	if(checkDefine.match(fileLines[1])):
		fileLines[1] = "#define " + guard
		numberDetect += 1

	if(checkEndif.match(fileLines[-2])):
		fileLines[-2] = "#endif // " + guard
		numberDetect += 1
	elif(checkEndif.match(fileLines[-1])):
		fileLines[-1] = "#endif // " + guard
		numberDetect += 1

	if(numberDetect != 3):
		print "File " + filename + " not fully modified: " + `numberDetect`
	else:
		with open(filename, 'w') as headerFile:
			for line in fileLines:
				headerFile.write(line + '\n')
		print "Written file " + filename
