#include <uls.h>

void mx_join(char **res, char *s2)
{
    char *newstr = mx_strnew(mx_strlen(*res) + mx_strlen(s2));
    int i = 0;
    char *s1 = *res;
    for(int j = 0; s1[j]; j++, i++)
    {
        newstr[i] = s1[j];
    }
    for(int j = 0; s2[j]; j++, i++)
    {
        newstr[i] = s2[j];
    }
    mx_strdel(&(*res));
    *res = newstr;
}

t_li *create_Flag_node(t_li *arg)
{
    t_li *node = (t_li *)malloc(sizeof (t_li));

    node->name = mx_strdup(arg->name);
    node->path = mx_strdup(arg->path);
    if (arg->err)
    {
        node->err = mx_strdup(arg->err);
    }
    else 
    {
        node->err = NULL;
    }
    lstat(node->path, &(node->info));
    if (arg->open != NULL)
    {
        node->open = arg->open;
    }
    else 
    {
        node->open = NULL;
    }
    return node;
}

void create_fde(t_li ***Flags, t_li ***dirs, t_li ***errors, t_li ***args)
{
    int j = 0;
    int nDir = 0;
    int nErr = 0;
    for (int i = 0; (*args)[i] != NULL; i++)
    {
        if ((*args)[i]->err == NULL)
        {
            if (!IS_DIR((*args)[i]->info.st_mode))
            {
                j++;
            } 
            else
            {
                nDir++;
            }
        } 
        else
        {
            nErr++;
        }
    }
    if (j > 0)
    {
        *Flags = malloc((j + 1) * sizeof(t_li *));
    }
    if (nDir > 0)
    {
        *dirs = malloc((nDir + 1) * sizeof(t_li *));
    }
    if (nErr > 0)
    {
        *errors = malloc((nErr + 1) * sizeof(t_li *));
    }
}

void fdir(t_li **args, s_type *num, t_li ***Flags, t_li ***dirs)
{
    if (!IS_DIR((*args)->info.st_mode))
    {
        (*Flags)[num->n_f++] = create_Flag_node((*args));
        (*Flags)[num->n_f] = NULL;
    }
    else
    {
        (*dirs)[num->n_d++] = create_Flag_node((*args));
        (*dirs)[num->n_d] = NULL;
    }
}

t_li **mx_get_Flags(t_li ***args, Flag *fl)
{
    t_li **Flags = NULL;
    t_li **dirs = NULL;
    t_li **errors = NULL;
    s_type *num = malloc(sizeof(s_type));
    num->n_d = 0;
    num->n_e = 0;
    num->n_f = 0;
    num->i = 0;
    create_fde(&Flags, &dirs, &errors, args);
    while ((*args)[num->i] != NULL)
    {
        if ((*args)[num->i]->err == NULL)
        {
            fdir(&(*args)[num->i], num, &Flags, &dirs);
        }
        else
        {
            errors[num->n_e++] = create_Flag_node((*args)[num->i]);
            errors[num->n_e] = NULL;
        }
        num->i++;
    }
    if (num->n_d > 1)
    {
        fl->Flags = 1;
    }
    free(num);
    return Flags;
}

int check_a(char *name, Flag *fl)
{
    if (fl->A != 1 || mx_strcmp(name, ".") == 0 || mx_strcmp(name, "..") == 0)
    {
        return 0;
    }
    return 1;
}

int count_read(t_li **arg, Flag *fl)
{
    int count = 0;
    t_li *args = *arg;
    DIR *dptr;
    struct dirent *ds;

    if (IS_DIR(args->info.st_mode) || IS_LNK(args->info.st_mode))
    {
        if ((dptr = opendir(args->path)) != NULL)
        {
            while ((ds = readdir(dptr)) != NULL)
            {
                if (ds->d_name[0] != '.' || check_a(ds->d_name, fl) == 1)
                {
                    count++;
                }
            }
            closedir(dptr);
        }
        else 
        {
            (*arg)->err = mx_strdup(strerror(errno));
            fl->ex = 1;
            return -1;
        }
    }
    return count;
}

t_li *create_he_node(char *name, char *path)
{
    t_li *node = (t_li *)malloc(sizeof(t_li));
    node->name = mx_strdup(name);
    node->path = mx_strdup(path);
    mx_join(&node->path, "/");
    mx_join(&node->path, name);
    node->err = NULL;
    if (lstat(node->path, &(node->info)) == -1)
    {
        node->err = mx_strdup(strerror(errno));
    }
    node->open = NULL;
    return node;
}

void open_dir(t_li ***args, Flag *fl)
{
    DIR *dptr;
    struct dirent *ds;
    int count = 0;

    for (int i = 0; (*args)[i] != NULL; i++)
    {
        count = count_read(&(*args)[i], fl);
        if (count > 0)
        {
            (*args)[i]->open = malloc((count + 1) * sizeof(t_li *));
            if ((dptr = opendir((*args)[i]->path)) != NULL)
            {
                for (count = 0; (ds = readdir(dptr)) != NULL;)
                {
                    if (ds->d_name[0] != '.' || check_a(ds->d_name, fl) == 1)
                    {
                        (*args)[i]->open[count++] = create_he_node(ds->d_name, (*args)[i]->path);
                    }
                }
                (*args)[i]->open[count] = NULL;
                closedir(dptr);
            }
        }
    }
    mx_out_put_all(args, fl);
}

void mx_opendir(t_li ***args, Flag *fl)
{
    t_li **Flags = mx_get_Flags(&(*args), fl);
	if (Flags) 
    {
		mx_out_put_menu(&Flags, fl, 0);
		if (*args)
        {
			mx_printchar('\n');
        }
		fl->Flags = 1;
	}
    if (*args)
    {
        open_dir(&(*args), fl);
    }
}

t_li *create_li_node(char *data)
{
    t_li *node = (t_li *)malloc(1 * sizeof(t_li));
    node->name = mx_strdup(data);
    node->path = mx_strdup(data);
    node->err = NULL;
    if (lstat(data, &(node->info)) == -1)
    {
        node->err = mx_strdup(strerror(errno));	
    }
    node->open = NULL;
    return node;
}

t_li **create_list(char **name, int count)
{
    t_li **new = malloc(count * sizeof(t_li *));
    int i = 0;
    while(name[i])
    {
        new[i] = create_li_node(name[i]);
        i++;
    }
    new[i] = NULL;
    return new;
}

char **names(int argc, char **argv, int i, int *count)
{
    int j = i;
    char **names = NULL;

    if (i == argc)
    {
        *count = 2;
        names = malloc(2 * sizeof(char *));
        names[0] = mx_strdup(".");
        names[1] = NULL;
    }
    else
    {
        while(argv[j])
        {
            j++;
        } 
        names = malloc((j - i + 1) * sizeof(char *));
        for(j = 0; argv[i]; i++, j++)
        {
            names[j] = mx_strdup(argv[i]);
        }
        names[j] = NULL;
        *count = j + 1;
    }
    return names;
}

t_li **mx_get_names(int argc, char **argv, int i) 
{
    int count = 0;
    char **name = names(argc, argv, i, &count);
    t_li **args = create_list(name, count);
    return args;
}
