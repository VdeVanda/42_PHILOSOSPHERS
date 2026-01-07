/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   creating.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vabatist <vabatist@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/17 12:08:52 by vabatist          #+#    #+#             */
/*   Updated: 2026/01/03 20:50:44 by vabatist         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/**
 * @brief Check if the variables provided are valid.
 * @param table Pointer to the table structure.
 * @param argv Array of argument strings.
 * @return true if all variables are valid, false otherwise.
 */
static bool	check_vars(t_table *table, char **argv)
{
	if (argv[5])
	{
		table->num_eat = ft_atol(argv[5]);
		if (table->num_eat <= 0 || table->num_eat > INT_MAX)
			return (exit_msg(table, "Invalid amount to eat\n", 0, 0), false);
	}
	if (table->num_philos <= 0 || table->num_philos > INT_MAX)
		return (exit_msg(table, "Invalid amount of philosophers\n", 0, 0),
			false);
	if (table->time_die <= 0 || table->time_die > INT_MAX)
		return (exit_msg(table, "Invalid time to die\n", 0, 0), false);
	if (table->time_eat <= 0 || table->time_eat > INT_MAX)
		return (exit_msg(table, "Invalid time to eat\n", 0, 0), false);
	if (table->time_sleep <= 0 || table->time_sleep > INT_MAX)
		return (exit_msg(table, "Invalid time to sleep\n", 0, 0), false);
	return (true);
}

/**
 * PRECONDITION: argv contains valid arguments.
 * @brief Create and initialize the table structure.
 * @param argv Array of argument strings.
 * num_philos: Number of philosophers. argv[1]
 * time_die: Time to die in milliseconds. argv[2]
 * time_eat: Time to eat in milliseconds. argv[3]
 * time_sleep: Time to sleep in milliseconds. argv[4]
 * num_eat: Number of times each philosopher must eat (optional). argv[5]
 * @return Pointer to the created table structure, or NULL on failure.
 */
t_table	*create_table(char **argv)
{
	t_table	*table;

	table = ft_calloc(1, sizeof(t_table));
	if (!table)
		return (exit_msg(NULL, "Failed malloc making_table\n", 0, 0), NULL);
	table->num_philos = ft_atol(argv[1]);
	table->time_die = ft_atol(argv[2]);
	table->time_eat = ft_atol(argv[3]);
	table->time_sleep = ft_atol(argv[4]);
	if (argv[5])
		table->num_eat = ft_atol(argv[5]);
	else
		table->num_eat = -1;
	table->sim_run = false;
	table->time_start = 0;
	if (check_vars(table, argv) == false)
		return (NULL);
	table->philos = ft_calloc(table->num_philos, sizeof(t_philo));
	if (!table->philos)
		return (exit_msg(table, "Failed malloc table->philos\n", 0, 0), NULL);
	table->forks = ft_calloc(table->num_philos, sizeof(pthread_mutex_t));
	if (!table->forks)
		return (exit_msg(table, "Failed malloc table->forks\n", 0, 0), NULL);
	return (table);
}

/**
 * @brief Create and initialize the philosophers.
 * @param table Pointer to the table structure.
 * First loop initializes each philosopher's ID, last meal (equals time start),
 * amount eaten, and table pointer. It also initializes each fork mutex.
 * Second loop assigns the left and right forks to each philosopher.
 * Last philosopher's right fork is the first fork (circular arrangement).
 * @return 1 on success, -1 on failure.
 */
int	create_philos(t_table *table)
{
	int	i;

	i = 0;
	if (init_mutexes(table) == false)
		return (-1);
	table->time_start = get_time(table);
	while (i < table->num_philos)
	{
		table->philos[i].id = i + 1;
		table->philos[i].last_meal = table->time_start;
		table->philos[i].eat_count = 0;
		table->philos[i].table = table;
		if (pthread_mutex_init(&table->forks[i], NULL) != 0)
			return (exit_msg(table, "Mutex init failed\n", 4, i), -1);
		i++;
	}
	i = 0;
	while (i < table->num_philos)
	{
		table->philos[i].left_f = &table->forks[i];
		table->philos[i].right_f = &table->forks[(i + 1) % table->num_philos];
		i++;
	}
	return (1);
}

/**
 * @brief Start the simulation by creating philosopher threads
 * and monitor thread.
 * @param table Pointer to the table structure.
 * Creates a monitor thread to oversee the simulation.
 * The monitor checks for dead philosophers and if all have eaten enough.
 * Creates a thread for each philosopher to run their routine.
 * routine is the function where philosopher eat/sleep/think actions occur.
 * When the simulation ends,
 * the monitor thread and philosopher threads are joined.
 * @return 1 on success, -1 on failure.
 */
int	start_routine(t_table *table)
{
	int			i;
	pthread_t	tmonitor;

	i = 0;
	table->sim_run = true;
	if (pthread_create(&tmonitor, NULL, monitor, table))
		return (exit_msg(table, "Failed to create thread\n", 4,
				table->num_philos), -1);
	while (i < table->num_philos)
	{
		if (pthread_create(&table->philos[i].thread, NULL, routine,
				&table->philos[i]) != 0)
			return (exit_msg(table, "Failed to create thread\n", 4,
					table->num_philos), -1);
		i++;
	}
	i = 0;
	pthread_join(tmonitor, NULL);
	while (i < table->num_philos)
	{
		pthread_join(table->philos[i].thread, NULL);
		i++;
	}
	return (1);
}
