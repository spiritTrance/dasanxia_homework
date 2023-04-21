class grammarAnalysiser(object):
    def __init__ (self, path: str):
        with open(path, encoding="utf8") as f:
            datas = f.readlines()
        signSet = set()
        self.sign2int = {}
        self.int2sign = {}
        self.grammar = {}
        self.count = 0
        for row in datas:
            row = row.strip("\n").strip(" ")
            if len(row) == 0 or row[0] == '#':
                continue
            row = row.split(' ')
            u = row[0]
            v = row[2].strip("[").strip("}").strip("{").strip("]")
            if not u in signSet:
                self.count += 1
                self.sign2int[u] = self.count
                self.int2sign[self.count] = u
                signSet.add(u)
            if not v in signSet:
                self.count += 1
                self.sign2int[v] = self.count
                self.int2sign[self.count] = v
                signSet.add(v)
            if self.grammar.get(self.sign2int[u]) is None:
                self.grammar[self.sign2int[u]] = [self.sign2int[v]]
            else:
                self.grammar[self.sign2int[u]].append(self.sign2int[v])
        for i in range(1, self.count + 1):
            if self.grammar.get(i) is not None:
                self.grammar[i] = list(set(self.grammar[i]))
                
    def isTerminal(self, s: str):
        return s.strip("'") != s or s in ["Ident", "IntConst", "floatConst"]
            
    def isTerminal(self, s: int):
        s = self.int2sign[s]
        return s.strip("'") != s or s in ["Ident", "IntConst", "floatConst"]
            
    def __findLeftRecursion(self, u):
        if self.vis[u] == 1:
            if not self.isTerminal(u):
                print(self.int2sign[u], "is left recursive!")
            return
        self.vis[u] = 1
        if not self.isTerminal(u):
            for i in self.grammar[u]:
                self.__findLeftRecursion(i)
        self.vis[u] = 2
        return


    def findLeftRecursion(self):
        self.vis = [0 for _ in range(self.count + 10)]
        for i in range(1, self.count + 1):
            if self.vis[i] == 0:
                self.__findLeftRecursion(i)
        self.vis = [0 for _ in range(self.count + 10)]
    
    def __findFirstSet(self, u):
        if self.isTerminal(u) :
            if self.firstSet.get(u) is None:
                self.firstSet[u] = set([u])
            return           
        us = set() 
        for v in self.grammar[u]:
            self.__findFirstSet(v)
            vs = self.firstSet[v]
            us.update(vs)
        if self.firstSet.get(u) is None:
            self.firstSet[u] = set()
        self.firstSet[u].update(us)
        return
    
    def __updateFirstSet(self, u):
        if self.isTerminal(u):
            return False
        prevLen = len(self.firstSet[u])
        for v in self.grammar[u]:
            self.firstSet[u].update(self.firstSet[v])
        nextLen = len(self.firstSet[u])
        return prevLen != nextLen       # True if update!
        
    def findFirstSet(self):
        self.firstSet = {}
        for i in range(1, self.count + 1):
            if self.firstSet.get(i) is None:
                self.__findFirstSet(i)
        while True:
            flag = False
            for i in range(1, self.count + 1):
                flag = flag or self.__updateFirstSet(i)
            if flag == False:
                break
            
    def determineFirstIntersection(self):
        for i in range(1, self.count + 1):
            if self.isTerminal(i):
                continue
            s = set(i for i in range(1, self.count + 1))
            for v in self.grammar[i]:
                s = s.intersection(self.firstSet[v])
                if len(self.grammar[i]) > 1:
                    print("\t",self.int2sign[i], self.int2sign[v], self.firstSet[v])
            if len(self.grammar[i]) > 1:
                print("Ans =", s)
        print(self.int2sign[8], self.int2sign[9])
        
    def printAllNonTreminalDecl(self):
        for i in range(1, self.count + 1):
            if not self.isTerminal(i):
                s = "bool parse{:s}({:s}* root);".format(self.int2sign[i], self.int2sign[i])
                print(s)
            
grammarAnalysiser = grammarAnalysiser("./test.txt")
grammarAnalysiser.findLeftRecursion()
grammarAnalysiser.findFirstSet()
# grammarAnalysiser.printAllNonTreminalDecl()
grammarAnalysiser.determineFirstIntersection()