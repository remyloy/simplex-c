#include "stdafx.h"

#define GRIDSIZE 10000
#define NUM_PARAMS 3
#define SIMPLEX_SIZE (NUM_PARAMS + 1)
#define IDX_BEST 0
#define IDX_SND_WORST (SIMPLEX_SIZE - 2)
#define IDX_WORST (SIMPLEX_SIZE - 1)

typedef struct tagContributionParameter
{
	double a;
	double b;
	double c;
} ContributionParameter;

typedef struct tagFitParameter
{
	double b1;
	double c1;
	double c2;
	double cost;
} FitParameter;

typedef struct tagSimplexParameter
{
	double α;
	double β;
	double γ;
	double σ;
} SimplexParameter;

static double grid[GRIDSIZE];
static double dataFunction[GRIDSIZE];
static double contribution1[GRIDSIZE];
static double contribution2[GRIDSIZE];
static double function[GRIDSIZE];

static FitParameter simplex[SIMPLEX_SIZE];

void evaluateContribution1(ContributionParameter ps)
{
	for (int i = 0; i < GRIDSIZE; i++)
	{
		contribution1[i] = ps.a + ps.b * grid[i] + ps.c * grid[i] * grid[i];
	}
}

void evaluateContribution2(ContributionParameter ps)
{
	for (int i = 0; i < GRIDSIZE; i++)
	{
		contribution2[i] = ps.a / (ps.b + grid[i]) + ps.c * sqrt(grid[i]);
	}
}

void evaluateFunction(FitParameter fitParameter)
{
	ContributionParameter ps1;
	ContributionParameter ps2;
	ps1.a = 1.0;
	ps1.b = fitParameter.b1;
	ps1.c = fitParameter.c1;
	ps2.a = 3.99;
	ps2.b = 5.0;
	ps2.c = fitParameter.c2;
	evaluateContribution1(ps1);
	evaluateContribution2(ps2);
	for (int i = 0; i < GRIDSIZE; i++)
	{
		function[i] = contribution1[i] + contribution2[i];
	}
}

void evaluateDataFunction()
{
	ContributionParameter ps1;
	ContributionParameter ps2;
	ps1.a = 1.0;
	ps1.b = 2.01;
	ps1.c = 3.01;
	ps2.a = 3.99;
	ps2.b = 5.0;
	ps2.c = 6.0;
	evaluateContribution1(ps1);
	evaluateContribution2(ps2);
	for (int i = 0; i < GRIDSIZE; i++)
	{
		dataFunction[i] = contribution1[i] + contribution2[i];
	}
}

double cost()
{
	double sum = 0.0;
	for (int i = 0; i < GRIDSIZE; i++)
	{
		double epsilon = dataFunction[i] - function[i];
		sum += epsilon * epsilon;
	}
	return sum;
}

void initSimplex(FitParameter ps)
{
	simplex[0] = ps;
	for (int i = 1; i < SIMPLEX_SIZE; i++)
	{
		simplex[i] = simplex[0];
		switch (i)
		{
		case 1:
			simplex[i].b1 += 0.5 * fabs(simplex[i].b1 * 0.5);
			break;
		case 2:
			simplex[i].c1 += 0.5 * fabs(simplex[i].c1 * 0.5);
			break;
		case 3:
			simplex[i].c2 += 0.5 * fabs(simplex[i].c2 * 0.5);
			break;
		}
	}
}

int compare(const void* a, const void *b) 
{
	return ((FitParameter*)a)->cost < ((FitParameter*)b)->cost ? -1 : 1;
}

void sort()
{
	qsort(simplex, SIMPLEX_SIZE, sizeof(FitParameter), compare);
}

FitParameter calculateMean() 
{
	FitParameter mean;
	mean.b1 = 0.0;
	mean.c1 = 0.0;
	mean.c2 = 0.0;
	for (int i = 0; i < NUM_PARAMS; i++)
	{
		mean.b1 += simplex[i].b1;
		mean.c1 += simplex[i].c1;
		mean.c2 += simplex[i].c2;
	}

	double f = 1.0 / NUM_PARAMS;
	mean.b1 *= f;
	mean.c1 *= f;
	mean.c2 *= f;
	return mean;
}

void reflect(double f, FitParameter mean, FitParameter* p)
{
	mean.b1 *= 1.0 + f;
	mean.c1 *= 1.0 + f;
	mean.c2 *= 1.0 + f;
	p->b1 *= f;
	p->c1 *= f;
	p->c2 *= f;

	p->b1 = mean.b1 - p->b1;
	p->c1 = mean.c1 - p->c1;
	p->c2 = mean.c2 - p->c2;	

	evaluateFunction(*p);
	p->cost = cost();
}

void expand(double f, FitParameter mean, FitParameter* p)
{
	mean.b1 *= 1.0 + f;
	mean.c1 *= 1.0 + f;
	mean.c2 *= 1.0 + f;
	p->b1 *= f;
	p->c1 *= f;
	p->c2 *= f;

	p->b1 = mean.b1 - p->b1;
	p->c1 = mean.c1 - p->c1;
	p->c2 = mean.c2 - p->c2;

	evaluateFunction(*p);
	p->cost = cost();
}

void contract(double f, FitParameter mean, FitParameter* p)
{
	mean.b1 *= f;
	mean.c1 *= f;
	mean.c2 *= f;
	p->b1 *= 1.0 - f;
	p->c1 *= 1.0 - f;
	p->c2 *= 1.0 - f;

	p->b1 = mean.b1 + p->b1;
	p->c1 = mean.c1 + p->c1;
	p->c2 = mean.c2 + p->c2;

	evaluateFunction(*p);
	p->cost = cost();
}

void compress(double f, FitParameter mean, FitParameter* p)
{
	mean.b1 *= f;
	mean.c1 *= f;
	mean.c2 *= f;
	p->b1 *= 1.0 - f;
	p->c1 *= 1.0 - f;
	p->c2 *= 1.0 - f;

	p->b1 = mean.b1 + p->b1;
	p->c1 = mean.c1 + p->c1;
	p->c2 = mean.c2 + p->c2;

	evaluateFunction(*p);
	p->cost = cost();
}

void step(SimplexParameter ps)
{
	sort();

	FitParameter mean = calculateMean();
	FitParameter reflected = simplex[IDX_WORST];
	reflect(ps.α, mean, &reflected);
	if (reflected.cost < simplex[IDX_BEST].cost)
	{
		FitParameter expanded = simplex[IDX_WORST];
		expand(ps.γ, mean, &expanded);

		simplex[IDX_WORST] = reflected.cost < expanded.cost ? reflected : expanded;
	}
	else if (reflected.cost < simplex[IDX_SND_WORST].cost)
	{
		simplex[IDX_WORST] = reflected;
	}
	else
	{
		FitParameter contracted = reflected.cost < simplex[IDX_WORST].cost ? reflected : simplex[IDX_WORST];
		contract(ps.β, mean, &contracted);
		if (contracted.cost < simplex[IDX_WORST].cost)
		{
			simplex[IDX_WORST] = contracted;
		}
		else
		{
			for (int i = 1; i < SIMPLEX_SIZE; i++)
			{
				compress(ps.σ, simplex[IDX_BEST], &simplex[i]);
			}
		}
	}
}

void integrationTest()
{	
	for (int i = 0; i < GRIDSIZE; i++)
	{
		grid[i] = i / 10000.0;
	}
	evaluateDataFunction();
	FitParameter initial;	
	initial.b1 = 4.01;
	initial.c1 = 5.01;
	initial.c2 = 8.0;
	initial.cost = 0.0;
	evaluateFunction(initial);
	initSimplex(initial);
	for (int i = 0; i < SIMPLEX_SIZE; i++)
	{
		evaluateFunction(simplex[i]);
		simplex[i].cost = cost();
	}

	SimplexParameter ps;
	ps.α = 1.0;
	ps.β = 0.5;
	ps.γ = 2.0;
	ps.σ = 0.5;
	for (int i = 0; i < 50; i++)
	{
		step(ps);
	}

	sort();
	//printf("%f %f %f %f\n", simplex[0].b1, simplex[0].c1, simplex[0].c2, simplex[0].cost);
}

int main()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	LARGE_INTEGER t1, t2;
	for (int i = 0; i < 100; i++)
	{
		QueryPerformanceCounter(&t1);
		integrationTest();
		QueryPerformanceCounter(&t2);
		double elapsed_secs = (t2.QuadPart - t1.QuadPart) * 1.0 / freq.QuadPart;
		printf("elapsed %f\n", elapsed_secs);
	}
	return 0;
}
