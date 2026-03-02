#include "contur/statistic.h"
#include <cmath>

Table::Table(int tExec, int tServ) : tExec(tExec), tServ(tServ)
{
}

int Table::getTime(StatTime time) const
{
    if (time == TimeExec) {
        return tExec;
    }
    if (time == TimeServ) {
        return tServ;
    }
    return -1;
}

void Statistic::setObservation(const std::string &user, int tExec, int tServ)
{
    hmIter = ovserv.find(user);
    if (hmIter == ovserv.end()) {
        std::vector<Table> tb;
        tb.push_back(Table(tExec, tServ));
        ovserv.insert(pairObserv(user, tb));
    } else {
        hmIter->second.push_back(Table(tExec, tServ));
    }
}

double Statistic::getTpredict(const std::string &user, StatTime time)
{
    hmIter = ovserv.find(user);
    if (hmIter == ovserv.end()) {
        return -1.0;
    }
    unsigned n = hmIter->second.size();
    if (n == 0) {
        return -1;
    }
    double alfa = 0.8;
    double Tpredict = alfa * hmIter->second[n - 1].getTime(time);
    for (unsigned i = n - 1; i > 0; i--) {
        Tpredict = Tpredict + pow((1 - alfa), i) * alfa * hmIter->second[i - 1].getTime(time);
    }
    return Tpredict;
}

double Statistic::getTpredictSimple(const std::string &user, StatTime time)
{
    if (hmIter == ovserv.end()) {
        return -1.0;
    }
    unsigned n = hmIter->second.size();
    double Tpredict = 0;
    for (unsigned i = 0; i < n; i++) {
        Tpredict = Tpredict + hmIter->second[i].getTime(time);
    }
    return Tpredict / n;
}

void Statistic::clearTpredict()
{
    ovserv.erase(ovserv.begin(), ovserv.end());
}

double Statistic::getTimeThreshold(StatTime time)
{
    int size_ = ovserv.size();
    double beta = 1 / (1 + pow(M_E, -size_));
    hmIter = ovserv.begin();
    double TpredictMin = getTpredict(hmIter->first, time);
    for (hmIter++; hmIter != ovserv.end(); hmIter++) {
        if (TpredictMin > getTpredict(hmIter->first, time)) {
            TpredictMin = getTpredict(hmIter->first, time);
        }
    }
    return TpredictMin * beta;
}
