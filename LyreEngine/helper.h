#pragma once

// don't use std localtime()! 
std::tm localtime_threadsafe(const time_t* _Time);

// get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string CurrentDateTimeStr();


