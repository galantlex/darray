#include "darray.h"
#include "dstring.h"

//////////////////////////////////// DARRAY ////////////////////////////////////
static inline void _da_memswap(void* p1, void* p2, size_t sz)
{
    char tmp, *a = p1, *b = p2;
    for (size_t i = 0; i < sz; ++i)
    {
        tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

void* da_alloc(size_t nelem, size_t size)
{
    size_t capacity = DA_NEW_CAPACITY_FROM_LENGTH(nelem);
    struct _darray* darr = malloc(sizeof(struct _darray) + capacity*size);
    if (darr == NULL)
        return darr;
    darr->_elemsz = size;
    darr->_length = nelem;
    darr->_capacity = capacity;
    return darr->_data;
}

void* da_alloc_exact(size_t nelem, size_t size)
{
    struct _darray* darr = malloc(sizeof(struct _darray) + nelem*size);
    if (darr == NULL)
        return darr;
    darr->_elemsz = size;
    darr->_length = nelem;
    darr->_capacity = nelem;
    return darr->_data;
}

void da_free(void* darr)
{
    free(DA_P_HEAD_FROM_HANDLE(darr));
}

size_t da_length(void* darr)
{
    return *DA_P_LENGTH_FROM_HANDLE(darr);
}

size_t da_capacity(void* darr)
{
    return *DA_P_CAPACITY_FROM_HANDLE(darr);
}

size_t da_sizeof_elem(void* darr)
{
    return *DA_P_SIZEOF_ELEM_FROM_HANDLE(darr);
}

void* da_resize(void* darr, size_t nelem)
{
    size_t new_capacity = DA_NEW_CAPACITY_FROM_LENGTH(nelem);
    size_t new_arr_size = sizeof(struct _darray) + new_capacity*da_sizeof_elem(darr);
    struct _darray* ptr = realloc(DA_P_HEAD_FROM_HANDLE(darr), new_arr_size);
    if (ptr == NULL)
        return NULL;
    ptr->_length = nelem;
    ptr->_capacity = new_capacity;
    return ptr->_data;
}

void* da_resize_exact(void* darr, size_t nelem)
{
    size_t new_arr_size = sizeof(struct _darray) + nelem*da_sizeof_elem(darr);
    struct _darray* ptr = realloc(DA_P_HEAD_FROM_HANDLE(darr), new_arr_size);
    if (ptr == NULL)
        return NULL;
    ptr->_length = nelem;
    ptr->_capacity = nelem;
    return ptr->_data;
}

void* da_reserve(void* darr, size_t nelem)
{
    size_t curr_capacity = da_capacity(darr);
    size_t min_capacity = da_length(darr) + nelem;
    if (curr_capacity >= min_capacity)
        return darr;
    size_t new_capacity = DA_NEW_CAPACITY_FROM_LENGTH(min_capacity);
    size_t new_arr_size = sizeof(struct _darray) + new_capacity*da_sizeof_elem(darr);
    struct _darray* ptr = realloc(DA_P_HEAD_FROM_HANDLE(darr), new_arr_size);
    if (ptr == NULL)
        return NULL;
    ptr->_capacity = new_capacity;
    return ptr->_data;
}

void da_swap(void* darr, size_t index_a, size_t index_b)
{
    size_t size = da_sizeof_elem(darr);
    _da_memswap(
        ((char*)darr) + (index_a * size),
        ((char*)darr) + (index_b * size),
        size
    );
}

void* da_cat(void* dest, void* src, size_t nelem)
{
    size_t offset = da_length(dest)*da_sizeof_elem(dest);
    dest = da_resize(dest, da_length(dest)+nelem);
    if (dest == NULL)
        return NULL;
    memcpy(dest+offset, src, nelem*da_sizeof_elem(dest));
    return dest;
}

/////////////////////////////////// DSTRING ////////////////////////////////////
darray(char) dstr_alloc_empty()
{
    char* darr = da_alloc(1, sizeof(char));
    darr[0] = '\0';
    return darr;
}

darray(char) dstr_alloc_from_cstr(const char* src)
{
    size_t src_len_with_nullterm = strlen(src)+1;
    char* darr = da_alloc(src_len_with_nullterm, sizeof(char));
    memcpy(darr, src, src_len_with_nullterm);
    return darr;
}

darray(char) dstr_alloc_from_dstr(const darray(char) src)
{
    size_t src_len_with_nullterm = da_length((void*)src);
    char* darr = da_alloc(src_len_with_nullterm, sizeof(char));
    memcpy(darr, src, src_len_with_nullterm);
    return darr;
}

void dstr_free(darray(char) dstr)
{
    da_free(dstr);
}

size_t dstr_length(const darray(char) dstr)
{
    // dstrings always have a null terminator so this should never underflow.
    return da_length((void*)dstr)-1;
}

darray(char) dstr_cat_cstr(darray(char) dest, const char* src)
{
    size_t dest_len = da_length((void*)dest)-1;
    size_t src_len_with_nullterm = strlen(src)+1;
    dest = da_resize(dest, dest_len+src_len_with_nullterm);
    if (dest == NULL)
        return NULL;
        memcpy(dest+dest_len, src, src_len_with_nullterm);
    return dest;
}

darray(char) dstr_cat_dstr(darray(char) dest, const darray(char) src)
{
    size_t dest_len = da_length((void*)dest)-1;
    size_t src_len_with_nullterm = da_length((void*)src);
    dest = da_resize(dest, dest_len+src_len_with_nullterm);
    if (dest == NULL)
    return NULL;
    memcpy(dest+dest_len, src, src_len_with_nullterm);
    return dest;
}

int dstr_cmp(const darray(char) s1, const char* s2)
{
    while (*s1 == *s2 && *s1 != '\0')
    {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int dstr_cmp_case(const darray(char) s1, const char* s2)
{
    while (tolower(*s1) == tolower(*s2) && *s1 != '\0')
    {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

long dstr_find(darray(char) dstr, const char* substr)
{
    char* loc = strstr(dstr, substr);
    if (loc == NULL)
        return -1;
    return loc - dstr;
}

long dstr_find_case(darray(char) dstr, const char* substr)
{
    char* loc = strcasestr(dstr, substr);
    if (loc == NULL)
        return -1;
    return loc - dstr;
}

darray(char) dstr_replace_all(darray(char) dstr, const char* substr,
    const char* new_substr)
{
    size_t substr_len = strlen(substr);
    size_t new_substr_len = strlen(new_substr);
    long loc;
    while ((loc = dstr_find(dstr, substr)) != -1)
    {
        da_remove_arr(dstr, loc, substr_len);
        if (!da_insert_arr(dstr, loc, new_substr, new_substr_len))
            return NULL;
    }
    return dstr;
}

darray(char) dstr_replace_all_case(darray(char) dstr, const char* substr,
    const char* new_substr)
{
    size_t substr_len = strlen(substr);
    size_t new_substr_len = strlen(new_substr);
    long loc;
    while ((loc = dstr_find_case(dstr, substr)) != -1)
    {
        da_remove_arr(dstr, loc, substr_len);
        if (!da_insert_arr(dstr, loc, new_substr, new_substr_len))
            return NULL;
    }
    return dstr;
}

void dstr_transform_lower(darray(char) dstr)
{
    da_foreach(dstr, c)
        *c = tolower(*c);
}

void dstr_transform_upper(darray(char) dstr)
{
    da_foreach(dstr, c)
        *c = toupper(*c);
}
