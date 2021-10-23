#ifndef GENERATOR_ASSIGNMENT
#define GENERATOR_ASSIGNMENT

#include "GeneratorContext.hpp"

void assignmentFromTree(generator::GeneratorContext& ctx,
                        const Node& root,
                        const Token& LHSToken,
                        const Token& RHSToken,
                        const Unit& LHS,
                        const Unit& RHS);

void assignment(generator::GeneratorContext& ctx, const Node& root);

#endif