import sys

def lensplit(s,maxlen):
    r = []
    while len(s)>maxlen:
            r.append(s[:maxlen])
            s = s[maxlen:]
    if len(s)>0:
            r.append(s)
    return r

def keytojava():
	if( len(sys.argv)<2 ):
		print("Usage:\n   " + sys.argv[0] + " <filename>\n")
		return
	with open(sys.argv[1]) as f:
		s = f.readline()
	es = s.replace('\\','\\\\').replace('"','\\"')
	result = '"' + '"\n + "'.join(lensplit(es,50)) + '"'
	print(result)

if __name__ == '__main__':
	keytojava()