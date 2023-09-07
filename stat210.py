## Spring 2023 Stat 210 Project 2
    ## Task: simulate "hot hand fallacy" using 20,000 randomly generated Monte Carlo Simulations to determine shooting percentage.

import random
#id: 3393123
#a = 1, b = 2, c = 3
    #shooting percentage: 53%
    #sequence length: 3

shootProb = 53

def produceSequence(p):
    seq = []
    for i in range(50):
        n = random.randint(0, 100)
        if n > p:
            seq.append(0)
        else:
            seq.append(1)
    return seq

def countStreaks(seq):
    count1 = 0
    count0 = 0
    counts = []
    for i in range(len(seq)):
        if i <= 2:
            pass
        elif seq[i-3] == seq[i-2] == seq[i-1] == seq[i] == 1:
            count1 = count1 + 1
        elif seq[i-3] == seq[i-2] == seq[i-1] == 1 and seq[i] == 0:
            count0 = count0 + 1
    counts.append(count0)
    counts.append(count1)
    return counts

def simulate():
    probs = []
    sum = 0
    avg = 0

    for i in range(10000):
        seqs = produceSequence(shootProb)
        counts = countStreaks(seqs)
        if not counts[1] == 0 and not counts[0] == 0:
            probs.append(counts[1]/(counts[0] + counts[1]))
        else:
            pass
    for i in range(len(probs)):
        sum = sum + probs[i]
    
    descStats = []
    return sum/len(probs)

print(simulate())

