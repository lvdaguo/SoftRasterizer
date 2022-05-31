#include "pch.h"
#include "Random.h"

std::random_device Random::rd = { };
std::mt19937 Random::rng = std::mt19937(rd());