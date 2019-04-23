// g++ particles.cpp -lm -lglut -lGL -lGLU -o particles
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <time.h>

// the GLUT and OpenGL libraries have to be linked correctly
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


#define EPSILON_0     8.85e-12
#define Q_ELECTRON    1.60e-19
#define LENGTH_SCALE  1e-6
#define TIME_SCALE    1e-10

#define X_SIZE 1000
#define Y_SIZE 1000

#define PI 3.14159265

class Vector2d {
public:
        // Atributes
        double x;
        double y;

        // Class Methods 
        Vector2d();
        Vector2d(double, double);
        Vector2d& operator += (Vector2d const &v);
        Vector2d& operator -= (Vector2d const &v);
        double magnitude();
        double argument();
        void zero();
};

Vector2d::Vector2d() {
        x = 0;
        y = 0;
}

Vector2d::Vector2d (double x_val, double y_val) {
        x = x_val;
        y = y_val;
}

Vector2d& Vector2d::operator += (Vector2d const &v) {
        this->x += v.x;
        this->y += v.y;
        return *this;
}

Vector2d& Vector2d::operator -= (Vector2d const &v) {
        this->x += v.x;
        this->y += v.y;
        return *this;
}

double Vector2d::magnitude() {
        return sqrt( this->x*this->x + this->y*this->y);
}

double Vector2d::argument() {
        return atan2(this->y, this->x);
}

void Vector2d::zero() {
        this->x = 0;
        this->y = 0;
}

Vector2d operator + (Vector2d const &v1, Vector2d const &v2) {
        return Vector2d(v1.x + v2.x, v1.y + v2.y);
}

Vector2d operator - (Vector2d const &v1, Vector2d const &v2) {
        return Vector2d(v1.x - v2.x, v1.y - v2.y);
}

Vector2d operator * (Vector2d const &v, double scale_factor) {
        return Vector2d(v.x*scale_factor, v.y*scale_factor);
}

Vector2d operator * (double scale_factor,Vector2d const &v) {
        return Vector2d(v.x*scale_factor, v.y*scale_factor);
}

double operator * (Vector2d const &v1, Vector2d const &v2) {
        return v1.x*v2.x + v1.y*v2.y;
}

Vector2d operator / (Vector2d const &v, double scale_factor) {
        return Vector2d(v.x/scale_factor, v.y/scale_factor);
}

Vector2d rotate(Vector2d const &v, float angle) {
        return Vector2d(cos(angle)*v.x-sin(angle)*v.y,
                        sin(angle)*v.x+cos(angle)*v.y);
}


class Particle {
public:
        Vector2d position;
        Vector2d velocity;
        double y;
        double m;
        double q;
        Particle(double, double, double, double);
        Particle(Vector2d, double, double);
};

Particle::Particle(Vector2d pos, double mass, double charge) {
        velocity.x = 0;
        velocity.y = 0;
        position = pos;
        m = mass;
        q = charge;
}

Particle::Particle(double x, double y, double mass, double charge) {
        velocity.x = 0;
        velocity.y = 0;
        position.x = x;
        position.y = y;
        m = mass;
        q = charge;
}


std::vector<Particle> particles;
std::vector< std::vector <Vector2d> > E_field(X_SIZE, std::vector <Vector2d> (Y_SIZE, Vector2d(0,0))); 

void calculate_e_field() {
        Vector2d E;
        Vector2d pos;
        Vector2d r;

        for(std::vector<double>::size_type x = 0; x != E_field.size(); x++) {
                pos.x = x;
                for(std::vector<int>::size_type y = 0; y != E_field.size(); y++) {
                        pos.y = y;
                        for(std::vector<int>::size_type i = 0; i != particles.size(); i++) {
                                r = pos - particles[i].position;
                                E += r * (particles[i].q/(EPSILON_0*pow(r.magnitude(),3)));
                        }
                        E_field[x][y] = E;
                }
        }
}

void calculate_velocities() {
        Vector2d r;
        Vector2d E;
        Vector2d Fb;
        double friction_coefficient = 0.0;
        float B = 100000000;
        for(std::vector<int>::size_type i = 0; i != particles.size(); i++) {
                E.zero();
                for(std::vector<int>::size_type j = 0; j != particles.size(); j++) {
                        if (i==j) {
                                continue;
                        }
                        r = particles[i].position - particles[j].position;
                        E += r * (particles[j].q/(EPSILON_0*pow(r.magnitude(),3)));
                }
                //Fb = B*particles[i].q*rotate(particles[i].velocity,PI/2);
                particles[i].velocity += (E*particles[i].q) * (TIME_SCALE / particles[i].m);
        }
}

double pixels[X_SIZE*Y_SIZE][3] = {};

void draw_field() {
        double colour = 0.0f;
        double max_value = 1e19;
        for(std::vector<double>::size_type x = 0; x != E_field.size(); x++) {
                for(std::vector<int>::size_type y = 0; y != E_field.size(); y++) {
                        if (E_field[x][y].magnitude() > 0 && E_field[x][y].magnitude() < max_value) {
                                colour = (log10(E_field[x][y].magnitude()+1)-12)/(log10(max_value)-12);
                                pixels[x*X_SIZE + y][0] = 0.0f;
                                pixels[x*X_SIZE + y][1] = colour;
                                pixels[x*X_SIZE + y][2] = 0.0f;
                                std::cout << E_field[x][y].magnitude() << " " << colour << std::endl;
                        } else {
                                pixels[x*X_SIZE + y][0] = 0.0f;
                                pixels[x*X_SIZE + y][1] = 0.0f;
                                pixels[x*X_SIZE + y][2] = 0.0f;
                        }
                }
        }
}

void draw_reg_polygon(double x0, double y0, int vertices, double radius)
{
        int i;
        double dx, dy, angle;
        double twicePi = 2.0 * 3.142;

        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x0, y0);
        for (i=0; i<vertices+1; i++) {
                angle = twicePi * (double) i / (double) vertices;
                dx = radius * sin(angle);
                dy = radius * cos(angle);
                glVertex2f(x0+dx, y0+dy);
        }
        glEnd();
}

/* Handler for window-repaint event. Call back when the window first appears and
   whenever the window needs to be re-painted. */
void display_particles() {
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
        //glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)

        //draw_field();
        //glRasterPos2i(-1, -1);
        //glDrawPixels(X_SIZE, Y_SIZE, GL_RGB, GL_FLOAT, pixels);

        for(std::vector<int>::size_type i = 0; i != particles.size(); i++) {
                double x_pos = particles[i].position.x/(X_SIZE/2.0f)-1.0f;
                double y_pos = particles[i].position.y/(Y_SIZE/2.0f)-1.0f;

                if (particles[i].q >= 0) {
                  glColor3f(1.0f, 0.0f, 0.0f); // Red
                } else {
                  glColor3f(0.0f, 0.0f, 1.0f); // Blue   
                }

                draw_reg_polygon(x_pos, y_pos, 10, 0.01f);
        }

        glFlush();  // Render now
}

void update_particles(int t) {
        //calculate_e_field();
        calculate_velocities();

        for(std::vector<int>::size_type i = 0; i != particles.size(); i++) {
                particles[i].position += particles[i].velocity;
        }

        display_particles();
        glutTimerFunc(1, update_particles, 0);
}

void init_particles_grid(int side_length)
{
        for (int i=0; i<side_length*side_length; i++) {
                int x = (X_SIZE/(side_length+1))*(i%side_length + 1);
                int y = (Y_SIZE/(side_length+1))*(((int)(i/side_length)) + 1);
                int m = rand() % 3 + 1;
                int q = rand() % 4 - 1;
                if (q<=0) {
                        q-=1;
                }
                particles.push_back(Particle(x,y,m,q));
        }       
}

void init_particles_random(int num)
{
        for(int i=0; i<num; i++) {
                int x = rand() % X_SIZE;
                int y = rand() % Y_SIZE;
                int m = rand() % 3 + 1;
                int q = rand() % 4 - 1;
                if (q<=0) {
                        q-=1;
                }

                particles.push_back(Particle(x,y,m,q));
        }        
}

/* Main function: GLUT runs as a console application starting at main()  */
int main(int argc, char** argv) {
        srand(time(NULL));

        //init_particles_random(20);
        init_particles_grid(5);

        glutInit(&argc, argv);                 // Initialize GLUT
        glutInitWindowSize(X_SIZE, Y_SIZE);   // Set the window's initial width & height
        glutCreateWindow("OpenGL Setup Test"); // Create a window with the given title
        glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
        glutDisplayFunc(display_particles); // Register display callback handler for window re-paint
        glutTimerFunc(25, update_particles, 1);
        glutMainLoop();           // Enter the event-processing loop
        return 0;
}
