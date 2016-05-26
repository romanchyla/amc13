
import sys

def compare(v, w):
    output = {}
    kv = {}
   
    with open(v, 'r') as NT:
	for line in NT:
            line = line.strip()
            if 'STATUS' not in line:
                continue
            k,v = line.split(': ')
            if k in kv:
                print 'Problem, key is already in the dictionary', k
            kv[k] = v

    with open(w, 'r') as WT:
        for line in WT:
            line = line.strip()
            if 'STATUS' not in line:
                continue
            k,v = line.split(': ')
            if k in kv:
                x = kv[k]
                if v != x:
               
                   output[k] = '{0} <> {1}'.format(x, v)           
                   
            else:
                 kv[k] = '[], {0}'.format(v)
                
                
    sorted_items = sorted(output.items(), key=lambda x: x[1])
    for k,v in sorted_items:	    		
        print '%40s    %s' % (k,v)
    
if __name__ == '__main__':
    compare(sys.argv[1], sys.argv[2])
