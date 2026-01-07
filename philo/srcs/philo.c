/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vabatist <vabatist@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/17 12:09:38 by vabatist          #+#    #+#             */
/*   Updated: 2026/01/03 20:51:52 by vabatist         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/**
 * PRECONDITION: time_sleep must be > 0.
 * @brief Make the philosopher sleep for a specified time.
 * @param philo Pointer to the philosopher structure.
 * @param time_sleep Time to sleep in milliseconds.
 * start: The start time in milliseconds.
 * curr_time: The current time in milliseconds.
 * elapsed: time elapsed since start in milliseconds.
 * Loops until the elapsed time reaches time_sleep,
 * checking every 100 microseconds if the simulation is still running.
 * If the simulation stops, it returns false.
 * @return true if the philosopher slept for the full duration,
 * false if the simulation stopped.
 */
bool	sleep_philo(t_philo *philo, long time_sleep)
{
	long	start;
	long	curr_time;
	long	elapsed;

	if (time_sleep <= 0)
		return (true);
	start = get_time(philo->table);
	curr_time = 0;
	elapsed = 0;
	while (elapsed < time_sleep)
	{
		curr_time = get_time(philo->table);
		elapsed = curr_time - start;
		pthread_mutex_lock(&philo->table->sim_run_m);
		if (philo->table->sim_run == false)
		{
			pthread_mutex_unlock(&philo->table->sim_run_m);
			return (false);
		}
		pthread_mutex_unlock(&philo->table->sim_run_m);
		usleep(100);
	}
	return (true);
}

/**
 * @brief Philosopher picks up forks.
 * @param philo Pointer to the philosopher structure.
 * Uses alternating lock order to prevent circular deadlock:
 * - Even philosophers (0, 2, 4... ): pick left fork first, then right fork
 * - Odd philosophers (1, 3, 5...):  pick right fork first, then left fork
 *
 * After acquiring each fork, attempts to print "has taken a fork".
 * The print function (philo_msg) checks if simulation is still running
 * (sim_run flag) BEFORE printing.  If simulation has ended:
 * - Does NOT print the message
 * - Releases the fork(s) already acquired
 * - Returns false to signal thread should exit
 *
 * Special case: If there is only one philosopher, they cannot acquire
 * two forks. The one_philo() check handles this by waiting until time_die
 * and then releasing the single fork.
 *
 * @return true if both forks were successfully acquired and announced.
 * @return false if simulation ended or special case detected
 * (thread should exit).
 */
static	bool	pick_forks(t_philo *philo)
{
	if (philo->id % 2 == 0)
		pthread_mutex_lock(philo->left_f);
	else
		pthread_mutex_lock(philo->right_f);
	if (philo_msg(philo, "has taken a fork\n", FORK) == false)
	{
		if (philo->id % 2 == 0)
			pthread_mutex_unlock(philo->left_f);
		else
			pthread_mutex_unlock(philo->right_f);
		return (false);
	}
	if (!one_philo(philo))
		return (false);
	if (philo->id % 2 == 0)
		pthread_mutex_lock(philo->right_f);
	else
		pthread_mutex_lock(philo->left_f);
	if (philo_msg(philo, "has taken a fork\n", FORK) == false)
	{
		drop_forks(philo);
		return (false);
	}
	return (true);
}

/**
 * PRECONDITION: Philosopher must have both forks acquired (via pick_forks).
 * @brief Philosopher eats.
 * @param philo Pointer to the philosopher structure.
 * Updates last_meal time and increments eat_count.
 * Prints "is eating" message and sleeps for time_eat duration.
 * If simulation ends during eating, drops forks and returns false.
 * @return true if eating completed, false if simulation ended.
 * POSTCONDITION: Both forks are released regardless of success or failure.
 */
bool	eating(t_philo *philo)
{
	pthread_mutex_lock(&philo->table->last_meal_m);
	philo->last_meal = get_time(NULL);
	pthread_mutex_unlock(&philo->table->last_meal_m);
	pthread_mutex_lock(&philo->table->eat_count_m);
	philo->eat_count++;
	pthread_mutex_unlock(&philo->table->eat_count_m);
	if (philo_msg(philo, "is eating\n", EAT) == false)
	{
		drop_forks(philo);
		return (false);
	}
	drop_forks(philo);
	return (true);
}

/**
 * PRECONDITION: Philosopher structure is properly initialized.
 * @brief Routine function for each philosopher thread: eat, sleep, think loop.
 * @param ph Pointer to the philosopher structure.
 * Even indexed philosophers wait initially to stagger actions.
 * Enters an infinite loop of:
 * - Picking up forks
 * - Eating
 * - Sleeping
 * - Thinking
 * If any action fails (e.g., simulation ends), the thread exits.
 * @return NULL on thread exit. Void functions allways return NULL.
 */
void	*routine(void *ph)
{
	t_philo	*philo;

	philo = (t_philo *)ph;
	if (philo->id % 2 == 0)
		sleep_philo(philo, philo->table->time_eat / 2);
	while (1)
	{
		if (pick_forks(philo) == false)
			return (NULL);
		if (eating(philo) == false)
			return (NULL);
		if (philo_msg(philo, "is sleeping\n", SLEEP) == false)
			return (NULL);
		if (philo_msg(philo, "is thinking\n", THINK) == false)
			return (NULL);
	}
	return (NULL);
}

/**
 * @brief Main function to initialize and start the philosopher simulation.
 * @param argc Argument count.
 * @param argv Argument array:
 * argv[1]: number_of_philosophers
 * argv[2]: time_to_die (milliseconds)
 * argv[3]: time_to_eat (milliseconds)
 * argv[4]: time_to_sleep (milliseconds)
 * argv[5]: [optional] number_of_times_each_philosopher_must_eat
 * Validates argument count (5 or 6).
 * Creates table structure from arguments.
 * Initializes philosophers and starts their routines.
 * Cleans up resources after simulation ends.
 * @return 0 on successful completion, 1 on error.
 */
int	main(int argc, char **argv)
{
	t_table	*table;

	table = NULL;
	if (argc < 5 || argc > 6)
		return (exit_msg(table, "Invalid amount of arguments\n", 0, 0), 1);
	table = create_table(argv);
	if (table == NULL)
		return (1);
	if (create_philos(table) == -1)
		return (1);
	if (start_routine(table) == -1)
		return (1);
	destroy_and_free(table, 4, table->num_philos);
	return (0);
}
