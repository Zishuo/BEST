#pragma once

typedef struct peer_pair {
    int peer_1;
    int peer_2;

    peer_pair(int p_1 = 0,int p_2 = 0) {
        peer_1 = p_1;
        peer_2 = p_2;
    }

    inline bool operator==(const peer_pair & inter)const {
        if (inter.peer_1 == peer_1 && inter.peer_2 == peer_2) {
            return true;
        } else if (inter.peer_1 == peer_2 && inter.peer_2 == peer_1) {
            return true;
        } else {
            return false;
        }
    }

    inline bool operator < (const  peer_pair & inter)const {
        if (*this == inter) {
            return false;
        }

        int A1,A2,B1,B2;
        if (peer_1 < peer_2) {
            A1 = peer_1;
            A2 = peer_2;
        } else {
            A1 = peer_2;
            A2 = peer_1;
        }

        if (inter.peer_1 < inter.peer_2) {
            B1 = inter.peer_1;
            B2 = inter.peer_2;
        } else {
            B1 = inter.peer_2;
            B2 = inter.peer_1;
        }

        if (A1 < B1) {
            return true;
        } else if (A1 == B1) {
            if (A2 < B2) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
} Pair;



