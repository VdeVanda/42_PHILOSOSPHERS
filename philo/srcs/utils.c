/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vabatist <vabatist@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/17 12:09:47 by vabatist          #+#    #+#             */
/*   Updated: 2026/01/03 21:04:23 by vabatist         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/**
 * @brief Destroys all mutexes and frees all allocated memory.
 *
 * Performs complete cleanup of the simulation by destroying pthread mutexes
 * (both table mutexes and fork mutexes) and freeing all dynamically
 * allocated memory (philos array, forks array, and table structure).
 *
 * @param table Pointer to the table structure.
 * @param destroy Destruction level for table mutexes (0-4):
 *        - 0: No table mutexes
 *        - 1: print_m only
 *        - 2: print_m, last_meal_m
 *        - 3: print_m, last_meal_m, eat_count_m
 *        - 4: All table mutexes (print_m, last_meal_m, eat_count_m, sim_run_m)
 * @param destroy_forks Number of fork mutexes to destroy.
 * Destroys the specified mutexes and frees the philosophers
 * and forks arrays, as well as the table structure itself.
 */
void	destroy_and_free(t_table *table, int destroy, int destroy_forks)
{
	int	i;

	i = 0;
	if (destroy >= 1)
		pthread_mutex_destroy(&table->print_m);
	if (destroy >= 2)
		pthread_mutex_destroy(&table->last_meal_m);
	if (destroy >= 3)
		pthread_mutex_destroy(&table->eat_count_m);
	if (destroy >= 4)
		pthread_mutex_destroy(&table->sim_run_m);
	while (i < destroy_forks)
	{
		pthread_mutex_destroy(&table->forks[i]);
		i++;
	}
	if (table && table->philos)
		free(table->philos);
	if (table && table->forks)
		free(table->forks);
	free(table);
}

/**
 * @brief Prints an error message to stderr if provided, then
 * 		calls destroy_and_free to clean up resources.
 *
 * This function handles error reporting and resource cleanup in a single call.
 * It prints the specified error message character-by-character to stderr (fd 2)
 * and then destroys mutexes and frees allocated memory based on the provided
 * destruction levels.
 *
 * @param table Pointer to the table structure (can be NULL).
 * 				If NULL, no cleanup is performed (only message printing).
 * @param which Error message string to print to stderr (can be NULL).
 * 				If NULL, no message is printed (only cleanup is performed).
 * @param destroy Destruction level for table mutexes (0-4).
 * @param destroy_forks Number of fork mutexes to destroy.
 *
 * @note This function does not terminate the program. The caller is responsible
 *       for returning an appropriate exit code after calling this function.
 */
void	exit_msg(t_table *table, char *which, int destroy, int destroy_forks)
{
	if (which)
		while (which && *which)
			write(2, which++, 1);
	if (table)
		destroy_and_free(table, destroy, destroy_forks);
}

/**
 * @brief Gets current timestamp in milliseconds.
 *
 * If a table pointer is provided, returns the elapsed time since
 * the simulation started. Otherwise, returns the absolute current time.
 *
 * @param table Pointer to the table structure. If NULL, returns absolute
 *              timestamp.  If not NULL, returns relative timestamp (milliseconds
 *              since table->time_start)
 *
 * - struct timeval is a struct defined in <sys/time.h> used to get current time.
 * - tv_sec represents seconds since Unix Epoch (1 Jan 1970 00:00:00 UTC).
 * - tv_usec represents microseconds (1 millionth of a second).
 * - long current_t stores the calculated timestamp in milliseconds.
 * - gettimeofday(&time, NULL) fills the timeval struct with the current time.
 * - current_t = (time.tv_sec * 1000) + (time.tv_usec / 1000) converts
 * the time to milliseconds.
 *
 * @return Timestamp in milliseconds:
 *         - If table == NULL:   absolute time (ms since 1 Jan 1970 00:00:00 UTC)
 *         - If table != NULL:  relative time (ms since simulation start)
 */
long	get_time(t_table *table)
{
	struct timeval	time;
	long			current_t;

	gettimeofday(&time, NULL);
	current_t = (time.tv_sec * 1000) + (time.tv_usec / 1000);
	if (!table)
		return (current_t);
	else
		return (current_t - table->time_start);
}

/**
 * @brief Converts a string to a long integer.
 *
 * This function mimics the behavior of the standard atoi function but
 * returns a long integer. It handles optional leading whitespace,
 * an optional '+' or '-' sign, and numeric digits. If the string
 * contains invalid characters or overflows, it returns 0.
 *
 * @param num Pointer to the null-terminated string to convert.
 * @return The converted long integer value. Returns 0 on invalid input
 *         or overflow.
 */
long	ft_atol(char *num)
{
	int		i;
	int		sign;
	long	result;

	sign = 1;
	i = 0;
	result = 0;
	while (num[i] && (num[i] == ' ' || (num[i] >= 9 && num[i] <= 13)))
		i++;
	if (num[i] == '+' || num[i] == '-')
	{
		if (num[i] == '-')
			sign = -1;
		i++;
	}
	while (num[i] && (num[i] >= '0' && num[i] <= '9'))
	{
		result = result * 10 + num[i] - '0';
		if (result > INT_MAX)
			break ;
		i++;
	}
	if (num[i] && (num[i] < '0' || num[i] > '9'))
		result = 0;
	return (result * sign);
}

/**
 * @brief Allocates memory for an array and initializes it to zero.
 *
 * This function mimics the behavior of the standard calloc function.
 * It allocates memory for an array of nmemb elements of size bytes each
 * and initializes all bytes to zero. If the allocation fails or if
 * the requested size would cause an overflow, it returns NULL.
 *
 * @param nmemb Number of elements to allocate.
 * @param size Size of each element in bytes.
 * @return Pointer to the allocated memory initialized to zero,
 *         or NULL if allocation fails or on overflow.
 */
void	*ft_calloc(size_t nmemb, size_t size)
{
	unsigned char	*p;
	size_t			total;
	size_t			i;

	if (nmemb != 0 && size > (size_t)(-1) / nmemb)
		return (NULL);
	total = nmemb * size;
	p = malloc(total);
	if (!p)
		return (NULL);
	i = 0;
	while (i < total)
		p[i++] = 0;
	return (p);
}
