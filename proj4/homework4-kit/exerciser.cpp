#include "exerciser.h"

void exercise(connection *C)
{
    query1(C,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
    query1(C,0,0,0,1,1,10,0,0,0,0,0,0,0,0,0,0,0,0); 
    query2(C, "Red");
    query3(C, "WakeForest");
    query4(C, "MA", "Maroon");
    query5(C, 12);
}
