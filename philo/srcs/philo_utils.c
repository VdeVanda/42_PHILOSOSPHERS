/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vabatist <vabatist@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/17 12:09:25 by vabatist          #+#    #+#             */
/*   Updated: 2026/01/03 20:51:52 by vabatist         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/**
 * PRECONDITION: Philosopher must have picked up one fork.
 * @brief Handle the case of a single philosopher.
 * @param philo Pointer to the philosopher structure.
 * If there is only one philosopher, they will pick up one fork
 * and then wait until they die, as they cannot eat.
 * @return true if there are multiple philosophers, false if only one.
 */
bool	one_philo(t_philo *philo)
{
	if (philo->table->num_philos == 1)
	{
		sleep_philo(philo, philo->table->time_die);
		pthread_mutex_unlock(philo->right_f);
		return (false);
	}
	return (true);
}

/**
 * PRECONDITION: Philosopher must have both forks acquired (via pick_forks).
 * @brief Drop both forks held by the philosopher.
 * @param philo Pointer to the philosopher structure.
 * Unlocks the left and right fork mutexes.
 * The order of unlocking depends on the philosopher's ID
 * (to avoid deadlock).
 * If the philosopher's ID is even, unlock right fork first.
 * If the philosopher's ID is odd, unlock left fork first.
 * @return void.
 */
void	drop_forks(t_philo *philo)
{
	if (philo->id % 2 == 0)
	{
		pthread_mutex_unlock(philo->right_f);
		pthread_mutex_unlock(philo->left_f);
	}
	else
	{
		pthread_mutex_unlock(philo->left_f);
		pthread_mutex_unlock(philo->right_f);
	}
}

/**
 * PRECONDITION: Philosopher must not already hold any forks.
 * @brief Print a message for the philosopher's action.
 * @param philo Pointer to the philosopher structure.
 * @param msg Message string to print.
 * @param msg_id Identifier for the type of message/action.
 * Locks the print mutex to ensure thread-safe printing.
 * Checks if the simulation is still running before printing.
 * Prints the message with a timestamp and philosopher ID.
 * Unlocks the print mutex after printing.
 * Next, depending on the msg_id:
 * EAT:   Sleeps for time_eat ms
 * SLEEP:  Sleeps for time_sleep ms
 * THINK:  If num_philos is ODD, sleeps for (time_eat * 2 - time_sleep) ms
 *    This extra thinking time is applied to ALL philosophers to prevent
 *    starvation by ensuring fair distribution of fork access
 * Other (FORK, DIED, THINK in even philosophers):
 * 		No sleep, returns immediately after printing
 * @return true if the message was printed and action completed,
 * false if the simulation has ended.
 */
bool	philo_msg(t_philo *philo, char *msg, int msg_id)
{
	pthread_mutex_lock(&philo->table->print_m);
	pthread_mutex_lock(&philo->table->sim_run_m);
	if (philo->table->sim_run == false)
	{
		pthread_mutex_unlock(&philo->table->sim_run_m);
		pthread_mutex_unlock(&philo->table->print_m);
		return (false);
	}
	pthread_mutex_unlock(&philo->table->sim_run_m);
	if (msg)
		printf(GREEN "%ld" RESET " %d %s", get_time(philo->table), philo->id,
			msg);
	pthread_mutex_unlock(&philo->table->print_m);
	if (msg_id == EAT)
		if (!sleep_philo(philo, philo->table->time_eat))
			return (false);
	if (msg_id == SLEEP)
		if (!sleep_philo(philo, philo->table->time_sleep))
			return (false);
	if (msg_id == THINK)
		if (philo->table->num_philos % 2 != 0)
			if (!sleep_philo(philo, philo->table->time_eat * 2
					- philo->table->time_sleep))
				return (false);
	return (true);
}

/**
 * @brief Initialize mutexes for the table structure.
 * @param table Pointer to the table structure.
 * Initializes the print, last_meal, full, and sim_run mutexes.
 * If any initialization fails, it calls exit_msg to clean up
 * and returns false.
 * @return true if all mutexes were initialized successfully,
 * false otherwise.
 */
bool	init_mutexes(t_table *table)
{
	if (pthread_mutex_init(&table->print_m, NULL) != 0)
	{
		exit_msg(table, "Mutex init failed\n", 1, -1);
		return (false);
	}
	if (pthread_mutex_init(&table->last_meal_m, NULL) != 0)
	{
		exit_msg(table, "Mutex init failed\n", 2, -1);
		return (false);
	}
	if (pthread_mutex_init(&table->eat_count_m, NULL) != 0)
	{
		exit_msg(table, "Mutex init failed\n", 3, -1);
		return (false);
	}
	if (pthread_mutex_init(&table->sim_run_m, NULL) != 0)
	{
		exit_msg(table, "Mutex init failed\n", 4, -1);
		return (false);
	}
	return (true);
}
