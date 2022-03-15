#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <SFML/Graphics.hpp>

struct Point
{
    float x, y;
};

void CalculateAngle(float Q, float Fg, float& angle, float& AccelAngle, float Ry, float friction)
{
    AccelAngle = Q * Fg * Ry;
    AccelAngle = (AccelAngle * M_PI) / 180;

    angle = (angle * friction) + AccelAngle;
}

//Rotate around arbitary point
void RotateAroundPoint(float point_Ax, float point_Ay, float& point_P1x, float& point_P1y, float angle)
{
    float sinAngle, cosAngle, TposX, TposY, RposX, RposY, FposX, FposY;

    sinAngle = sin(angle);
    cosAngle = cos(angle);

    TposX = point_P1x - point_Ax;
    TposY = point_P1y - point_Ay;

    RposX = (TposX * cosAngle) - (TposY * sinAngle);
    RposY = (TposX * sinAngle) + (TposY * cosAngle);

    FposX = RposX + point_Ax;
    FposY = RposY + point_Ay;

    point_P1x = FposX;
    point_P1y = FposY;
}

void CalculateRy(float point_P1y, float point_P0y, float& Ry)
{
    Ry = point_P1y - point_P0y;
}

//Moves the ball along the ground, based where the end of the arm is
void newBallPos(float length, Point& Ball, Point P2, int radius, float& d)
{
    d = Ball.y - radius - P2.y;
    float w = sqrt(pow(length, 2) - pow(d, 2));
    Ball.x = P2.x - w;
}


//Calculates the angle between two vectors
void getVectorAngle(sf::Vector2f prevJ, sf::Vector2f currentJ, float& angleAccel)
{
    float mag1 = sqrt(pow(prevJ.x, 2) + pow(prevJ.y, 2));
    float mag2 = sqrt(pow(currentJ.x, 2) + pow(currentJ.y, 2));

    angleAccel = acos(((prevJ.x * currentJ.x) + (prevJ.y * currentJ.y)) / (mag1 * mag2));

}



int main()
{
    sf::RenderWindow window(sf::VideoMode(1024, 768), "Trebuchet Implementation: A0021070");

    ////////////////////////////////////
    /////Sets up the arm positions/////
    //////////////////////////////////

    Point A;
    A.x = 700.0f;
    A.y = 497.0f;
    
    Point P1;
    P1.x = 650.0f;
    P1.y = 480.0f;
    
    Point P0;
    P0.x = 650.0f;
    P0.y = 481.0f;
    
    Point P2;
    P2.x = 800.0f;
    P2.y = 530.0f;
    
    //make stand and ground lines
    Point Stick;
    Stick.x = 700.0f;
    Stick.y = 575.0f;
    
    Point Ground1;
    Ground1.x = 0.0f;
    Ground1.y = 575.0f;
    
    Point Ground2;
    Ground2.x = 1000.0f;
    Ground2.y = 575.0f;
    
    //set up ball
    int radius = 5;
    float ballAngleAccel = 0;
    sf::CircleShape circle(radius);
    
    sf::Vector2f prevJ;
    sf::Vector2f currentJ;
    
    //ball position
    Point Ball;
    Ball.x = 725;
    Ball.y = Ground2.y - radius;
    
    Point slingEnd = Ball;


    //ball properties
    circle.setOrigin(radius, radius);
    circle.setFillColor(sf::Color(150, 50, 250));

    float Ry = P1.y - P0.y;

    float totalAngle = 0.0f;
    float angle = 0.0f;
    float AccelAngle = 0.0f;
    float friction = 0.99f;

    float Fg = 1.0f;
    float Q = 0.0958f;

    sf::Vector2f ballVelocity = sf::Vector2f(-6, -6);
    float gravity = 0.11;
    float maxGravity = 5;

    float NewAngle = 0.0f;

    bool ballReleased = false;

    //Sets the arm starting position
    RotateAroundPoint(A.x, A.y, P1.x, P1.y, (30 * M_PI) / 180);
    RotateAroundPoint(A.x, A.y, P0.x, P0.y, (30 * M_PI) / 180);
    RotateAroundPoint(A.x, A.y, P2.x, P2.y, (30 * M_PI) / 180);
    CalculateRy(P1.y, P0.y, Ry);

    //Sling length
    float length = sqrt(pow(P2.y - Ball.y, 2) + pow(P2.x - Ball.x, 2));
    float d = 0;

    prevJ.x = Ball.x - P2.x;
    prevJ.y = Ball.y - P2.y;

  

    sf::Clock gameClock;


    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            default:
                break;
            }
        }
        float deltaTime = gameClock.getElapsedTime().asMilliseconds();
        if (deltaTime >= 20)
        {
            //Moving the arm
            CalculateAngle(Q, Fg, angle, AccelAngle, Ry, friction);
            RotateAroundPoint(A.x, A.y, P1.x, P1.y, angle);
            RotateAroundPoint(A.x, A.y, P0.x, P0.y, angle);
            RotateAroundPoint(A.x, A.y, P2.x, P2.y, angle);
            CalculateRy(P1.y, P0.y, Ry);

            totalAngle += angle;
           
            if (totalAngle * (180 / M_PI) > -65 && !ballReleased)
            {
                //While the ball is on the ground
                if (d < length - 5)
                {
                    newBallPos(length, Ball, P2, radius, d);
                    circle.setPosition(Ball.x, Ball.y);
                    slingEnd = Ball;

                    //Set current vector to P2->Ball
                    currentJ.x = Ball.x - P2.x;
                    currentJ.y = Ball.y - P2.y;
                    //Compare current vector to previous vector to calculate acceleration angle
                    getVectorAngle(prevJ, currentJ, ballAngleAccel);

                    prevJ = currentJ;

                }

                else if (d >= length - 5)
                {
                    //When the ball leaves the ground
                    RotateAroundPoint(A.x, A.y, Ball.x, Ball.y, angle);
                    circle.setPosition(Ball.x, Ball.y);

                    //Rotate the ball around the arm using the ball acceleration angle
                    RotateAroundPoint(P2.x, P2.y, Ball.x, Ball.y, -ballAngleAccel);
                    circle.setPosition(Ball.x, Ball.y);

                    slingEnd = Ball;
                }
            }
            
            else
            {
                //After the ball leaves the sling
                ballReleased = true;

                //Ball hits the ground
                if (Ball.y > Ground1.y - radius)
                {
                    ballVelocity.y = 0;
                    Ball.x += ballVelocity.x;
                    Ball.y = Ground1.y - radius;
                }
                else
                {
                    //Gives ball starting velocity 
                    Ball.x += ballVelocity.x;
                    Ball.y += ballVelocity.y;

                    //Adds gravity 
                    ballVelocity.y += gravity;
                }
                

                circle.setPosition(Ball.x, Ball.y);

                
            }
           

            window.clear();



/////////////////////////////////////////
////////////Drawing lines with SFML/////
///////////////////////////////////////


            //Arm render
            sf::Vertex line[] =
            {
                sf::Vertex(sf::Vector2f(P1.x, P1.y)),
                sf::Vertex(sf::Vector2f(P2.x, P2.y))
            };

            window.draw(line, 2, sf::Lines);

            //Trebuchet stand
            sf::Vertex Stand[] =
            {
                sf::Vertex(sf::Vector2f(A.x, A.y)),
                sf::Vertex(sf::Vector2f(Stick.x, Stick.y))
            };

            window.draw(Stand, 2, sf::Lines);


            //The ground
            sf::Vertex Ground[] =
            {
                sf::Vertex(sf::Vector2f(Ground1.x, Ground1.y)),
                sf::Vertex(sf::Vector2f(Ground2.x, Ground2.y))
            };

            window.draw(Ground, 2, sf::Lines);


            //Draw sling while ball is attached
            if (!ballReleased)
            {
                sf::Vertex string[] =
                {
                    sf::Vertex(sf::Vector2f(P2.x, P2.y)),
                    sf::Vertex(sf::Vector2f(slingEnd.x, slingEnd.y))
                };

                window.draw(string, 2, sf::Lines);
            }
           


            window.draw(circle);
            // Get the window to display its contents
            window.display();

            gameClock.restart();
        }
    }
          


    return 0;
}
