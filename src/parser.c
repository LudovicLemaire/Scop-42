// gcc -I./include parser.c -o parser && ./parser
#include <stdio.h>
#include <GL/gl3w.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <wow.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <unistd.h>

int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void print_vertex(t_vec *vertex, int size) {
    int i = 0;
    while (i < size) {
        printf("vertex[%d]: %f %f %f\n", i, vertex[i].x, vertex[i].y, vertex[i].z);
        ++i;
    }
}

void print_buffer(GLfloat *buffer, int size) {
    int i = 0;
    while (size--) {
        printf("%f, %f, %f, %f, %f, %f\n", buffer[i], buffer[i+1], buffer[i+2], buffer[i+3], buffer[i+4], buffer[i+5]);
        i += 6;
    }
}

void getMinMax(GLfloat *buffer, int size, t_obj_spec *mm) {
    int i = 0;

    while (size--) {
        mm->x_max = fmax(mm->x_max, buffer[i]);
        mm->x_min = fmin(mm->x_min, buffer[i]);
        mm->y_max = fmax(mm->y_max, buffer[i+1]);
        mm->y_min = fmin(mm->y_min, buffer[i+1]);
        mm->z_max = fmax(mm->z_max, buffer[i+2]);
        mm->z_min = fmin(mm->z_min, buffer[i+2]);
        i += 6;
    }
}

void getLengthMiddle(t_obj_spec *mm) {
    mm->x_length = fabs(mm->x_min - mm->x_max);
    mm->x_center = (mm->x_max + mm->x_min) / 2;
    mm->y_length = fabs(mm->y_min - mm->y_max);
    mm->y_center = (mm->y_max + mm->y_min) / 2;
    mm->z_length = fabs(mm->z_min - mm->z_max);
    mm->z_center = (mm->z_max + mm->z_min) / 2;
}

void fill_vertex(t_vec *vertex, float x, float y, float z) {
    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
}

float copyGetRC() {
    float color = (float)rand() / (float)RAND_MAX;
    return color;
}

char* concat(const char *s1, const char *s2)
{
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1);
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);
    return result;
}

void setFace(GLfloat **buffer, int *j, int *face_inc, GLfloat x, GLfloat y, GLfloat z, float r, float g, float b) {
    (*buffer)[(*j)++] = x;
    (*buffer)[(*j)++] = y;
    (*buffer)[(*j)++] = z;
    (*buffer)[(*j)++] = r;
    (*buffer)[(*j)++] = g;
    (*buffer)[(*j)++] = b;
    ++(*face_inc);
}

void parser(GLfloat **buffer, int *sizeMallocFaces, char *filename, t_obj_spec *mm) {
    srand(time(NULL));
    if (access(filename, F_OK ) != 0 || !EndsWith(filename, ".obj")) {
        printf("\x1b[91mPath\x1b[0m: %s\n", filename);
        exit(-1);
    } else {
        printf("\x1b[92mPath\x1b[0m: %s\n", filename);
    }

    FILE *inputfile = fopen(filename, "r");

    struct stat file_info;
    lstat(filename, &file_info);
    int sizeMallocVertex = file_info.st_size / 50;
    *sizeMallocFaces = sizeMallocVertex * 6;

    t_vec *vertex = calloc(sizeMallocVertex, sizeof(t_vec));
    *buffer = calloc(*sizeMallocFaces, sizeof(GLfloat));

    size_t linesize = 0;
    char* linebuf = 0;
    ssize_t linelen = 0;
    float x, y, z;
    int a_point, a_texture, a_normal, b_point, b_texture, b_normal, c_point, c_texture, c_normal;
    int vertex_inc = 0;
    int face_inc = 0;

    while ((linelen = getline(&linebuf, &linesize, inputfile)) > 0) {
        // realloc
        if (vertex_inc + 1 >= sizeMallocVertex) {
            sizeMallocVertex *= 1.5;
            printf("sizeMallocVertex: %d\n", sizeMallocVertex);
            vertex = realloc(vertex, (sizeMallocVertex) * sizeof(t_vec));
        }
        if ((face_inc + 1) * 18 >= *sizeMallocFaces) {
            *sizeMallocFaces *= 1.5;
            printf("\x1b[93msizeMallocFaces:\x1b[0m %d\n", *sizeMallocFaces);
            *buffer = realloc(*buffer, *sizeMallocFaces * sizeof(GLfloat));
        }
        // vertex
        if (sscanf(linebuf, "v %f %f %f", &x, &y, &z) == 3) {
            fill_vertex(&vertex[vertex_inc], x, y, z);
            ++vertex_inc;
        }

        // faces
        float r = copyGetRC();
        float g = copyGetRC();
        float b = copyGetRC();
        int j = face_inc * 6;
        if (sscanf(linebuf, "f %d/%d/%d %d/%d/%d %d/%d/%d", &a_point, &a_texture, &a_normal, &b_point, &b_texture, &b_normal, &c_point, &c_texture, &c_normal) == 9) {
            setFace(buffer, &j, &face_inc, vertex[a_point-1].x, vertex[a_point-1].y, vertex[a_point-1].z, r, g, b);
            setFace(buffer, &j, &face_inc, vertex[b_point-1].x, vertex[b_point-1].y, vertex[b_point-1].z, r, g, b);
            setFace(buffer, &j, &face_inc, vertex[c_point-1].x, vertex[c_point-1].y, vertex[c_point-1].z, r, g, b);
        }
        if (sscanf(linebuf, "f %d %d %d", &a_point, &b_point, &c_point) == 3) {
            setFace(buffer, &j, &face_inc, vertex[a_point-1].x, vertex[a_point-1].y, vertex[a_point-1].z, r, g, b);
            setFace(buffer, &j, &face_inc, vertex[b_point-1].x, vertex[b_point-1].y, vertex[b_point-1].z, r, g, b);
            setFace(buffer, &j, &face_inc, vertex[c_point-1].x, vertex[c_point-1].y, vertex[c_point-1].z, r, g, b);
        }
        if (sscanf(linebuf, "f %d/%d %d/%d %d/%d", &a_point, &a_texture, &b_point, &b_texture, &c_point, &c_texture) == 6) {
            setFace(buffer, &j, &face_inc, vertex[a_point-1].x, vertex[a_point-1].y, vertex[a_point-1].z, r, g, b);
            setFace(buffer, &j, &face_inc, vertex[b_point-1].x, vertex[b_point-1].y, vertex[b_point-1].z, r, g, b);
            setFace(buffer, &j, &face_inc, vertex[c_point-1].x, vertex[c_point-1].y, vertex[c_point-1].z, r, g, b);
        }
        if (sscanf(linebuf, "f %d//%d %d//%d %d//%d", &a_point, &a_normal, &b_point, &b_normal, &c_point, &c_normal) == 6) {
            setFace(buffer, &j, &face_inc, vertex[a_point-1].x, vertex[a_point-1].y, vertex[a_point-1].z, r, g, b);
            setFace(buffer, &j, &face_inc, vertex[b_point-1].x, vertex[b_point-1].y, vertex[b_point-1].z, r, g, b);
            setFace(buffer, &j, &face_inc, vertex[c_point-1].x, vertex[c_point-1].y, vertex[c_point-1].z, r, g, b);
        }

        free(linebuf);
        linebuf = NULL;
    }
    free(linebuf);
    linebuf = NULL;
    free(vertex);
    vertex = NULL;
    *buffer = realloc(*buffer, ((face_inc * 6) + 1) * sizeof(GLfloat));

    printf("\x1b[92mNombre de triangles:\x1b[0m %d\n", face_inc/3);

    mm->x_min = FLT_MAX;
    mm->y_min = FLT_MAX;
    mm->z_min = FLT_MAX;
    mm->x_max = -FLT_MAX;
    mm->y_max = -FLT_MAX;
    mm->z_max = -FLT_MAX;

    getMinMax(*buffer, face_inc, mm);
    getLengthMiddle(mm);
    //print_vertex(vertex, vertex_inc);
    //print_buffer(*buffer, face_inc);
}
