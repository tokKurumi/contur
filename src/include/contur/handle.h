#pragma once

#include <cstdlib>
#include <iostream>

class Handle
{
  public:
    Handle();
    virtual ~Handle();

    int getTenter() const
    {
        return Tenter;
    }
    int getTbegin() const
    {
        return Tbegin;
    }
    int getTservice() const
    {
        return Tservice;
    }
    int getTterminate() const
    {
        return Tterminate;
    }
    int getTexec() const
    {
        return Texec;
    }
    int getTround() const
    {
        return Texec + Tservice;
    }

    float getTnorm() const
    {
        return static_cast<float>(Texec + Tservice) / Tservice;
    }

    void ProcessTime();

  private:
    int ID;
    int Tenter;
    int Tbegin;
    int Tservice;
    int Tterminate;
    int Texec;

  protected:
    int getID() const
    {
        return ID;
    }
    void setID(int ID)
    {
        this->ID = ID;
    }
    void setTenter(int Tenter)
    {
        this->Tenter = Tenter;
    }
    void setTbegin(int Tbegin)
    {
        this->Tbegin = Tbegin;
    }
    void setTservice(int Tservice)
    {
        this->Tservice = Tservice;
    }
    void setTterminate(int Tterminate)
    {
        this->Tterminate = Tterminate;
    }
    void setTexec(int Texec)
    {
        this->Texec = Texec;
    }
    void clearTime();
};

class CS : public Handle
{
  public:
    CS();
    virtual ~CS();
    bool getCS() const
    {
        return cs;
    }
    void setCS(bool cs)
    {
        this->cs = cs;
    }

  private:
    bool cs;
};
