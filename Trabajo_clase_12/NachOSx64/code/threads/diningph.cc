#include "diningph.h"

DiningPh::DiningPh() {

    dp = new Lock( "dp" );

    for ( int i = 0; i < 5; i++ ) {
        self[ i ] = new Semaphore( "me",0);
        state[ i ] = Thinking;
    }

}


DiningPh::~DiningPh() {
    for(int i = 0; i<5; i++){
        delete self[i];
    }
    delete dp;
}


void DiningPh::pickup( long who ) {

    dp->Acquire();

    state[ who ] = Hungry;
    test( who );
    dp->Release();
    self[ who ]->P();
}


void DiningPh::putdown( long who ) {

    dp->Acquire();

    state[ who ] = Thinking;
    test( (who + 4) % 5 );
    test( (who + 1) % 5 );

    dp->Release();

}


void DiningPh::test( long i ) {

    if ( ( state[ (i + 4) % 5 ] != Eating ) && 
         ( state[ i ] == Hungry ) && 
         ( state[ (i + 1) % 5] != Eating ) ) {
        state[ i ] = Eating;
        self[ i ]->V();
    }

}

void DiningPh::print() {

    for ( int i = 0; i < 5; i++ ) {
        printf( "Philosopher %d is %s \n", i + 1, (state[i]==Hungry)?"Hungry":(state[i]==Thinking)?"Thinking":"Eating");

    }

}

