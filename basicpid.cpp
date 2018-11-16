#include "basicpid.hpp"

namespace terraclear
{
    basicpid::basicpid(double Kp, double Ki, double Kd, uint32_t interval_ms)
    {

    }

    basicpid::~basicpid() 
    {
    
    }
    
    double basicpid::domain_transform(double from_value, double from_min, double from_max, double to_min,  double to_max)
    {
        //constrain input value to fit between from_max and from_min
        double from_constrained = std::min(std::max(from_min, from_value), from_max);
        
        //rebase input and output values to be a zero based scale. 
        double from_scale = from_max - from_min;
        double to_scale = to_max - to_min;
        
        //rebase the constrained value and prevent a divide by zero if from_value was constrained to from_min
        double from_rebased =  (from_constrained <= from_min) ? from_constrained : from_constrained - from_min;

        //do the [from -> to] domain transform..
        double retval = ((from_rebased / from_scale) * to_scale) + to_min;

        return retval;
    }

    void basicpid::start()
    {
        
    }
    
    void basicpid::stop()
    {
        
    }
    
    double basicpid::compute(double Input, double Setpoint)
    {
        return 0;
    }
}

