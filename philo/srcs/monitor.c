/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vabatist <vabatist@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/17 12:09:11 by vabatist          #+#    #+#             */
/*   Updated: 2026/01/03 20:51:52 by vabatist         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/**
 * @brief Check if philosopher i hasn't eaten within time_to_die and stop the
 * simulation if so.
 * @param table Pointer to the table structure.
 * @param run Pointer to the simulation running flag.
 * @param i Index of the philosopher to check.
 * pthread_mutex_lock on last_meal_m to safely read last_meal.
 * pthread_mutex_lock on sim_run_m to safely update sim_run.
 * pthread_mutex_lock on print_m to safely print the death message.
 * @return void.
 */
void	dead_philo(t_table *table, bool *run, int i)
{
	pthread_mutex_lock(&table->last_meal_m);
	if (get_time(NULL) - table->philos[i].last_meal > table->time_die)
	{
		*run = false;
		pthread_mutex_lock(&table->sim_run_m);
		table->sim_run = false;
		pthread_mutex_unlock(&table->sim_run_m);
		pthread_mutex_lock(&table->print_m);
		printf(RED "%ld" RESET " %d died\n", get_time(table),
			table->philos[i].id);
		pthread_mutex_unlock(&table->print_m);
	}
	pthread_mutex_unlock(&table->last_meal_m);
}

/**
 * @brief Check if all philosophers have eaten enough times and stop the
 * simulation if so.
 * @param table Pointer to the table structure.
 * @param run Pointer to the simulation running flag.
 * @param i Index of the philosopher to check.
 * @param full Pointer to the count of philosophers who have eaten enough.
 * pthread_mutex_lock on eat_count_m to safely read eat_count.
 * pthread_mutex_lock on sim_run_m to safely update sim_run.
 * @return void.
 */
void	full_philo(t_table *table, bool *run, int i, int *full)
{
	pthread_mutex_lock(&table->eat_count_m);
	if (table->num_eat != -1 && table->philos[i].eat_count >= table->num_eat)
		(*full)++;
	pthread_mutex_unlock(&table->eat_count_m);
	if ((*full) == table->num_philos)
	{
		pthread_mutex_lock(&table->sim_run_m);
		table->sim_run = false;
		pthread_mutex_unlock(&table->sim_run_m);
		*run = false;
	}
}

/**
 * @brief Monitor thread function to oversee the simulation.
 * @param ph Pointer to the table structure.
 * @note The monitor is needed because threads can't easily watch themselves
 * - they're all busy doing their own work (eating, sleeping, thinking).
 * You need one dedicated observer to check the "big picture" and enforce rules.
 * Continuously checks for dead philosophers and if all have eaten enough.
 * table is the main structure containing simulation data.
 * run is a boolean flag to track if the simulation should continue.
 * usleep for CPU efficiency.
 * Without usleep, the loop would spin millions of times per second.
 * with usleep still checks frequently enough to catch deaths quickly.
 * @return NULL when the simulation ends.
 */
void	*monitor(void *ph)
{
	t_table	*table;
	bool	run;
	int		i;
	int		full;

	run = true;
	table = (t_table *)ph;
	while (1)
	{
		full = 0;
		i = 0;
		while (i < table->num_philos)
		{
			dead_philo(table, &run, i);
			full_philo(table, &run, i, &full);
			if (run == false)
				return (NULL);
			i++;
		}
		usleep(100);
	}
}
