#ifndef VIEWER_HPP
#define VIEWER_HPP

//-----------------------------------------------------------------------------

class viewer
{
    double port[4];

    double curr_c[3];
    double curr_p[3];
    double curr_v[3];
    double curr_r[2];
    double curr_z;

    double dest_c[3];
    double dest_p[3];
    double dest_v[3];
    double dest_r[2];
    double dest_z;

public:

    viewer(double, double, double, double);

    void move(const double[3]);
    void turn(double, double);
    void zoom(double);

    void pick(double[3], double[3]);
    void view(double[4][4]);

    void step(double);
    void draw();
};

//-----------------------------------------------------------------------------

#endif
