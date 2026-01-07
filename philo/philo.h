/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vabatist <vabatist@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/17 12:11:33 by vabatist          #+#    #+#             */
/*   Updated: 2026/01/03 20:51:52 by vabatist         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include "limits.h"
# include "pthread.h"
# include "stdbool.h"
# include "stdio.h"
# include "stdlib.h"
# include "sys/time.h"
# include "unistd.h"

# define THINK 1
# define FORK 2
# define EAT 3
# define SLEEP 4

# define RED "\033[31m"
# define GREEN "\033[32m"
# define RESET "\033[0m"

/**
 * @brief Philosopher structure representing each philosopher.
 * Contains philosopher ID, last meal time, amount eaten,
 * pointers to left and right fork mutexes, thread, and
 * pointer to the shared table structure.
 */
typedef struct s_philo
{
	int				id;
	long			last_meal;
	int				eat_count;
	pthread_mutex_t	*left_f;
	pthread_mutex_t	*right_f;
	pthread_t		thread;
	struct s_table	*table;
}					t_philo;

/**
 * @brief Table structure representing the dining table.
 * Contains arrays of philosophers and fork mutexes,
 * various mutexes for synchronization, simulation parameters,
 * and simulation state.
 */
typedef struct s_table
{
	t_philo			*philos;
	pthread_mutex_t	*forks;
	pthread_mutex_t	last_meal_m;
	pthread_mutex_t	print_m;
	pthread_mutex_t	sim_run_m;
	pthread_mutex_t	eat_count_m;
	long			num_philos;
	long			time_eat;
	long			time_die;
	long			time_sleep;
	long			num_eat;
	long			time_start;
	bool			sim_run;
}					t_table;

long				ft_atol(char *num);
void				*ft_calloc(size_t nmemb, size_t size);
t_table				*create_table(char **argv);
int					create_philos(t_table *table);
bool				init_mutexes(t_table *table);
int					start_routine(t_table *table);
void				*routine(void *ph);
void				*monitor(void *ph);
long				get_time(t_table *table);
bool				one_philo(t_philo *philo);
void				drop_forks(t_philo *philo);
bool				philo_msg(t_philo *philo, char *msg, int msg_id);
bool				sleep_philo(t_philo *philo, long time_sleep);
void				destroy_and_free(t_table *table, int destroy,
						int destroy_forks);
void				exit_msg(t_table *table, char *which, int destroy,
						int destroy_forks);

#endif
