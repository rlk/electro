#ifndef GALAXY_HPP
#define GALAXY_HPP

//-----------------------------------------------------------------------------

class node;

typedef node *node_p;

//-----------------------------------------------------------------------------

class star
{
    std::string name;

public:

    void draw() const;
};

typedef std::list<star>                 star_l;
typedef std::list<star>::const_iterator star_i;

//-----------------------------------------------------------------------------

class edge
{
    star_i i;
    star_i j;

public:

    void draw() const;
};

typedef std::list<edge>                 edge_l;
typedef std::list<edge>::const_iterator edge_i;

//-----------------------------------------------------------------------------

class cons
{
    std::string name;

    edge_i E;

public:

    void draw() const;
};

typedef std::list<cons>                 cons_l;
typedef std::list<cons>::const_iterator cons_i;

//-----------------------------------------------------------------------------

class node
{
    node_p N[8];

public:

    void view(const double[4][4]);
    void draw() const;
};

//-----------------------------------------------------------------------------

class galaxy
{
    star_l S;
    cons_l C;
    node_p N;

public:

    void pick(const double[3], const double[3]);
    void view(const double[4][4]);

    void step(double);
    void draw() const;

};

//-----------------------------------------------------------------------------

#endif
