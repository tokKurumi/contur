#pragma once

#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

#include "contur/architecture_cpu.h"

class Table
{
  public:
    Table(int tExec, int tServ);
    int getTime(StatTime time) const;

  private:
    int tExec;
    int tServ;
};

class Statistic
{
  public:
    Statistic()
    {
    }

    void setObservation(const std::string &user, int tExec, int tServ);
    double getTpredict(const std::string &user, StatTime time);
    double getTpredictSimple(const std::string &user, StatTime time);
    void clearTpredict();
    double getTimeThreshold(StatTime time);

  private:
    std::unordered_map<std::string, std::vector<Table>> ovserv;
    std::unordered_map<std::string, std::vector<Table>>::iterator hmIter;
    typedef std::pair<std::string, std::vector<Table>> pairObserv;
};
