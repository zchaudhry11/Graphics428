using UnityEngine;
using System.Collections;

public class MoveTo : MonoBehaviour
{
    //Animation
    private float currSpeed = 0.1f;
    private bool isMoving = false;
    private bool jumped = false;
    private float jumpTimer = 0.6f;
    private float jumpItr = 0;
    private OffMeshLinkData currentLink;
    private OffMeshLinkData emptyLink;

    private float startTime;
    private float fracJourney = 0.0f;

    private bool doubleClicked = false;
    private float doubleClickTimer = 1.5f;

    //Navigation
    private NavMeshAgent navAgent;
    private bool select;
    private bool set = false;
    private Vector3 MoveD;

	void Start ()
    {
        startTime = Time.time;

        navAgent = GetComponent<NavMeshAgent>();
        navAgent.autoTraverseOffMeshLink = false;
	}

    void FixedUpdate()
    {
        if (isMoving)
        {
            if (doubleClicked)
            {
                print("Double clicked!");
                navAgent.speed = 3.5f;
                if (currSpeed < 8)
                {
                    currSpeed += 0.1f;
                }
            }
            else
            {
                if (currSpeed < 7)
                {
                    currSpeed += 0.1f;
                    navAgent.speed = 1;
                }
            }
        }

        if (doubleClickTimer > 0)
        {
            doubleClickTimer -= Time.deltaTime;
        }
        else if (doubleClickTimer <= 0)
        {
            doubleClicked = false;
        }

        this.GetComponent<Animator>().SetFloat("Velocity", currSpeed);
    }

	void Update ()
    {
        if (Input.GetMouseButtonDown(1))
        {
            if (doubleClickTimer > 0)
            {
                doubleClicked = true;
            }
            doubleClickTimer = 1.5f;
        }


        if (select && set)
        {
            navAgent.destination = MoveD;
        }
        if (navAgent.pathStatus == NavMeshPathStatus.PathComplete)
        {
            set = false;
        }
        if (navAgent.pathPending == false) //Path created
        {
            if (navAgent.remainingDistance <= navAgent.stoppingDistance)
            {
                if (navAgent.hasPath || navAgent.velocity.sqrMagnitude == 0)
                {
                    if (currSpeed > 0.1f)
                    {
                        currSpeed -= 0.15f;
                    }
                    else if (currSpeed < 0.1f)
                    {
                        currSpeed = 0.1f;
                    }
                }
            }
            if (navAgent.remainingDistance <= 0)
            {
                isMoving = false;
            }
        }

        if (navAgent.isOnOffMeshLink && navAgent.currentOffMeshLinkData.endPos != currentLink.endPos)
        {
            if (jumped == false)
            {
                transform.gameObject.GetComponent<Animator>().SetBool("Jumped", true);
                jumpTimer = 0.6f;
                currentLink = navAgent.currentOffMeshLinkData;
                jumped = true;
            }
        }
        
        //Jumping
        if (jumpTimer > 0 && jumped)
        {
            jumpTimer -= Time.deltaTime;
            jumpItr += Time.deltaTime;

            fracJourney = jumpItr / 0.6f;
            
            if (fracJourney <= 1)
            {
                transform.position = Vector3.Lerp(currentLink.startPos, currentLink.endPos, fracJourney);
                transform.position = new Vector3(transform.position.x, transform.position.y+(Mathf.Sin(Mathf.PI * fracJourney)/2), transform.position.z) ;
            }
        }

        if (jumpTimer <= 0 && jumped)
        {
            jumpItr = 0;
            currSpeed = 0.1f;
            transform.gameObject.GetComponent<Animator>().SetBool("Jumped", false);
            transform.position = currentLink.endPos;
            jumped = false;
            navAgent.CompleteOffMeshLink();
            navAgent.Resume();
            currentLink = emptyLink;
        }
    }

    void Select(int x)
    {
        Debug.Log("Player Logged");
        select = true;
    }

    void Deselect(int x)
    {
        Debug.Log("Player Delogged");
        select = false;
    }

    void Destination (Vector3 dest)
    {
        Debug.Log("Destination Set");
        isMoving = true;
        MoveD = dest;
        set = true;
    }
    
}
