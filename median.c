#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define SPHERE 1
#define PLANE 2
#define DEG 1
#define RAD 2
#define GREATCIRCLE 1
#define EUCLIDEAN 2
#define MANHATTAN 3
#define WEIGHTED 1
#define UNWEIGHTED 2
#define SCHEDULED 1
#define UNSCHEDULED 2

//Only change these directives
#define SURFACE SPHERE
#define SPHERE_RADIUS 6367.704 //Mean Earth radius = 6371.0 km
#define ANGLE_UNIT DEG
#define DISTANCE MANHATTAN
#define PRECISION_MAX 0.000001
#define DISTANCE_UNIT_LABEL "km"
#define MEDIAN WEIGHTED
#define WEIGHTS SCHEDULED
#define NUM_WEEKDAYS 7
#define NUM_TIMES 4
#define VERBOSITY 1

#if SURFACE == SPHERE
#define R SPHERE_RADIUS
#define POINTS_UNIT_LABEL "rad"
#elif SURFACE == PLANE
#define R 1
#define POINTS_UNIT_LABEL DISTANCE_UNIT_LABEL
#endif

typedef struct {
	int n;
	int openTimes[NUM_TIMES];
	int closeTimes[NUM_TIMES];
} schedule;

typedef struct {
	long double x;
	long double y;
	long double w;
	schedule schedules[NUM_WEEKDAYS];
} point;

long double toRad(long double deg);
long double toDeg(long double rad);
long double distEuclidean(long double x1, long double y1, long double x2, long double y2);
long double distGreatCircle(long double lat1, long double lon1, long double lat2, long double lon2, long double r);
long double distManhattan(long double x1, long double y1, long double x2, long double y2, long double r);

int main(int argc, char **argv) {
	point *points = NULL;
	int n = 0;
	long double  x, y, w = 1, sumW = 0;
	long double xMin = INT_MAX, xMax = -INT_MAX, yMin = INT_MAX, yMax = -INT_MAX, prec;
	long double xC, xL, xR, yC, yL, yR;
	long double sumSW, sumNW, sumSE, sumNE, sumMin;
	int d, t, open;
	char line[LINE_MAX], *p;
	int i, j, k;
	FILE *f;

	f = fopen(argv[1], "r");
	#if MEDIAN == WEIGHTED
	while(fscanf(f, "%Lf %Lf %Lf", &x, &y, &w) != EOF) {
	#elif MEDIAN == UNWEIGHTED
	while(fscanf(f, "%Lf %Lf", &x, &y) != EOF) {
	#endif
		#if SURFACE == SPHERE && ANGLE_UNIT == DEG
		x = toRad(x);
		y = toRad(y);
		#endif
		if(x < xMin) {
			xMin = x;
		} else if(x > xMax) {
			xMax = x;
		}
		if(y < yMin) {
			yMin = y;
		} else if(y > yMax) {
			yMax = y;
		}
		sumW += w;
		points = realloc(points, (n+1)*sizeof(point));
		points[n].x = x;
		points[n].y = y;
		points[n].w = w+1;
		n++;
	}
	fclose(f);
	for(i = 0; i < n; i++) {
		points[i].w /= sumW;
	}

	#if MEDIAN == WEIGHTED && WEIGHTS == SCHEDULED
	sscanf(argv[4], "%d", &d);
	sscanf(argv[5], "%d", &t);
	for(i = 0; i < n; i++) {
		for(j = 0; j < NUM_WEEKDAYS; j++) {
			points[i].schedules[j].n = 0;
			for(k = 0; k < NUM_TIMES; k++) {
				points[i].schedules[j].openTimes[k] = -1;
				points[i].schedules[j].closeTimes[k] = -1;
			}
		}
	}
	#if VERBOSITY == 1
	printf("Schedules:\n");
	#endif
	f = fopen(argv[3], "r");
	for(i = 0; i < n; i++) {
		fgets(line, LINE_MAX, f);
		p = line;
		#if VERBOSITY == 1
		printf("%d\n", i+1);
		#endif
		while(j = (int)strtol(p, &p, 10)) {
			j--;
			points[i].schedules[j].openTimes[points[i].schedules[j].n] = (int)strtol(p, &p, 10);
			points[i].schedules[j].closeTimes[points[i].schedules[j].n] = (int)strtol(p, &p, 10);
			#if VERBOSITY == 1
			printf("Weekday %d: %d-%d\n", j+1,
			       points[i].schedules[j].openTimes[points[i].schedules[j].n],
			       points[i].schedules[j].closeTimes[points[i].schedules[j].n]);
			#endif
			(points[i].schedules[j].n)++;
		}
		for(k = 0, open = 0; k < points[i].schedules[d-1].n && !open; k++) {
			if(t >= points[i].schedules[d-1].openTimes[k] && t < points[i].schedules[d-1].closeTimes[k]) {
				open = 1;
			}
		}
		if(!open) {
			#if VERBOSITY == 1
			printf("Closed on weekday %d at %d\n", d, t);
			#endif
			points[i].w = 0;
		}

	}
	fclose(f);
	#if VERBOSITY == 1
	printf("\n");
	#endif
	#endif

	#if VERBOSITY == 1
	printf("Points:\n");
	for(i = 0; i < n; i++) {
		printf("%d (%Lf,%Lf) %s, weight %Lf\n", i+1, points[i].x, points[i].y, POINTS_UNIT_LABEL, points[i].w);
	}
	printf("Number of points: %d\n", n);
	printf("\n");
	#endif

	xC = (xMin+xMax)/2;
	yC = (yMin+yMax)/2;
	#if DISTANCE == GREATCIRCLE
	prec = distGreatCircle(xMin, yMin, xMax, yMax, R);
	#elif DISTANCE == EUCLIDEAN
	prec = distEuclidean(xMin, yMin, xMax, yMax);
	#elif DISTANCE == MANHATTAN
	prec = distManhattan(xMin, yMin, xMax, yMax, R);
	#endif
	#if VERBOSITY == 1
	printf("Starting at (%Lf, %Lf) %s\n", xC, yC, POINTS_UNIT_LABEL);
	printf(" Precision: %Lf %s\n", prec, DISTANCE_UNIT_LABEL);
	#endif
	while(prec > PRECISION_MAX) {
		xL = (xMin + xC)/2;
		xR = (xC + xMax)/2;
		yL = (yMin + yC)/2;
		yR = (yC + yMax)/2;
		sumSW = sumNW = sumSE = sumNE = 0;

		for(i = 0; i < n; i++) {
			#if DISTANCE == GREATCIRCLE
                	sumSW += distGreatCircle(xL, yL, points[i].x, points[i].y, R)*(points[i].w);
			sumNW += distGreatCircle(xL, yR, points[i].x, points[i].y, R)*(points[i].w);
                        sumSE += distGreatCircle(xR, yL, points[i].x, points[i].y, R)*(points[i].w);
			sumNE += distGreatCircle(xR, yR, points[i].x, points[i].y, R)*(points[i].w);
			#elif DISTANCE == EUCLIDEAN
			sumSW += distEuclidean(xL, yL, points[i].x, points[i].y)*(points[i].w);
			sumNW += distEuclidean(xL, yR, points[i].x, points[i].y)*(points[i].w);
			sumSE += distEuclidean(xR, yL, points[i].x, points[i].y)*(points[i].w);
			sumNE += distEuclidean(xR, yR, points[i].x, points[i].y)*(points[i].w);
			#elif DISTANCE == MANHATTAN
		  	sumSW += distManhattan(xL, yL, points[i].x, points[i].y, R)*(points[i].w);
			sumNW += distManhattan(xL, yR, points[i].x, points[i].y, R)*(points[i].w);
                        sumSE += distManhattan(xR, yL, points[i].x, points[i].y, R)*(points[i].w);
			sumNE += distManhattan(xR, yR, points[i].x, points[i].y, R)*(points[i].w);
			#endif
                }
		#if VERBOSITY == 1
		printf("Moving ");
		#endif
		if(sumSW <= sumNW && sumSW <= sumSE && sumSW <= sumNE) {
			#if VERBOSITY == 1
			printf("SW ");
			#endif
			sumMin = sumSW;
			xMax = xC;
			yMax = yC;
		} else if(sumNW <= sumSW && sumNW <= sumSE && sumNW <= sumNE) {
	       		#if VERBOSITY == 1
      			printf("NW ");
			#endif
			sumMin = sumNW;
			xMax = xC;
	                yMin = yC;
       		} else if(sumSE <= sumSW && sumSE <= sumNW && sumSE <= sumNE) {
			#if VERBOSITY == 1
			printf("SE ");
			#endif
	                sumMin = sumSE;
			xMin = xC;
	                yMax = yC;
	        } else if(sumNE <= sumSW && sumNE <= sumNW && sumNE <= sumSE) {
			#if VERBOSITY == 1
			printf("NE ");
			#endif
			sumMin = sumNE;
	                xMin = xC;
	                yMin = yC;
	        } else {
			#if VERBOSITY == 1
			printf("nowhere ");
			#endif
		}

		xC = (xMin+xMax)/2;
		yC = (yMin+yMax)/2;
		#if DISTANCE == GREATCIRCLE
		prec = distGreatCircle(xMin, yMin, xMax, yMax, R);
		#elif DISTANCE == EUCLIDEAN
		prec = distEuclidean(xMin, yMin, xMax, yMax);
		#elif DISTANCE == MANHATTAN
		prec = distManhattan(xMin, yMin, xMax, yMax, R);
		#endif
		#if VERBOSITY == 1
		printf("to (%Lf, %Lf) %s\n", xC, yC, POINTS_UNIT_LABEL);
		printf(" Precision: %Lf %s\n", prec, DISTANCE_UNIT_LABEL);
		printf(" Sum of distances%s: %Lf %s\n", MEDIAN == WEIGHTED ? " (weighted)" : "", sumMin, DISTANCE_UNIT_LABEL);
		#endif
	}
	free(points);

	#if VERBOSITY == 1
        printf("Stopped at (%Lf, %Lf) %s", xC, yC, POINTS_UNIT_LABEL);
	#if SURFACE == SPHERE && ANGLE_UNIT == DEG
	printf(" = (%Lf, %Lf) deg", toDeg(xC), toDeg(yC));
	#endif
	printf("\n");
        printf(" Precision: %Lf %s\n", prec, DISTANCE_UNIT_LABEL);
	printf(" Sum of distances%s: %Lf %s\n", MEDIAN == WEIGHTED ? " (weighted)" : "", sumMin, DISTANCE_UNIT_LABEL);
	printf("\n");
	#endif
	#if VERBOSITY == 1
	printf("SURFACE: ");
	#if SURFACE == SPHERE
	printf("SPHERE\n");
	printf("SPHERE_RADIUS: %f %s\n", SPHERE_RADIUS, DISTANCE_UNIT_LABEL);
	printf("ANGLE_UNIT: ");
		#if ANGLE_UNIT == DEG
		printf("DEG\n");
		#elif ANGLE_UNIT == RAD
		printf("RAD\n");
		#endif
	#elif SURFACE == PLANE
	printf("PLANE\n");
	#endif
	printf("DISTANCE: ");
	#if DISTANCE == EUCLIDEAN
	printf("EUCLIDEAN\n");
	#elif DISTANCE == GREATCIRCLE
	printf("GREATCIRCLE\n");
	#elif DISTANCE == MANHATTAN
	printf("MANHATTAN\n");
	#endif
	printf("PRECISION_MAX: %f %s\n", PRECISION_MAX, DISTANCE_UNIT_LABEL);
	printf("MEDIAN: ");
	#if MEDIAN == WEIGHTED
	printf("WEIGHTED\n");
	#elif MEDIAN == UNWEIGHTED
	printf("UNWEIGHTED\n");
	#endif
	printf("WEIGHTS: ");
	#if WEIGHTS == SCHEDULED
	printf("SCHEDULED\n");
	#elif WEIGHTS == UNSCHEDULED
	printf("UNSCHEDULED\n");
	#endif
	#endif
	f = fopen(argv[2], "a");
	fprintf(f, "%Lf,%Lf/", toDeg(xC), toDeg(yC));
	fclose(f);
	return 0;
}

long double toRad(long double deg) {
	return 2*M_PI/360*deg;
}

long double toDeg(long double rad) {
	return 360/(2*M_PI)*rad;
}

long double distEuclidean(long double x1, long double y1, long double x2, long double y2) {
	return sqrt(pow(x1-x2, 2) + pow(y1-y2, 2));
}

long double distGreatCircle(long double lat1, long double lon1, long double lat2, long double lon2, long double r) {
	long double dLat, dLon, dS;

	dLat = fabs(lat1-lat2);
        dLon = fabs(lon1-lon2);
        dS = 2*asin(sqrt(pow(sin(dLat/2), 2) + cos(lat1)*cos(lat2)*pow(sin(dLon/2), 2)));
        return r*dS;
}

long double distManhattan(long double x1, long double y1, long double x2, long double y2, long double r) {
	return (fabs(x1-x2) + fabs(y1-y2))*r;
}
