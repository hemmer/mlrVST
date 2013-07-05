/*
  ==============================================================================

    Array2D.h
    Created: 5 Jul 2013 8:27:24pm
    Author:  hemmer

  ==============================================================================
*/

#ifndef __ARRAY2D_H_6CADCB82__
#define __ARRAY2D_H_6CADCB82__

#include "../JuceLibraryCode/JuceHeader.h"


template <class T>

class Array2D

{

public:

    Array2D(const int &r, const int &c, const T &defaultValue) :
        arrayData(), numRows(r), numCols(c)
    {
        setSize(numRows, numCols, defaultValue);
    }

    void setSize(const int &r, const int &c, const T &defaultValue)
    {
        numRows = r; numCols = c;

        arrayData.clear();
        arrayData.insertMultiple(0, defaultValue, numRows*numCols);
    }

    const T get(const int &i, const int &j)
    {
        const int index = i*numCols + j;
        return arrayData[index];
    }

    void set(const int &i, const int &j, const T &newValue)
    {
        const int index = i*numCols + j;
        arrayData.set(index, newValue);
    }

private:

    Array<T> arrayData;
    int numRows, numCols;

};



#endif  // __ARRAY2D_H_6CADCB82__
